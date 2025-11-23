#include "pch.h"

#include "D3D12Multithreading.h"

#include "FrameResource.h"

#include "CompiledShaders/SpaceObjectsPS.h"
#include "CompiledShaders/SpaceObjectsVS.h"

D3D12Multithreading* D3D12Multithreading::s_app = nullptr;
bool D3D12Multithreading::s_bIsEnhancedBarriersEnabled = false;

D3D12Multithreading::D3D12Multithreading()
  : m_frameIndex(0), m_fenceEvent(nullptr), m_fenceValue(0), m_rtvDescriptorSize(0), m_keyboardInput(),
    m_currentFrameResourceIndex(0), m_pCurrentFrameResource(nullptr)
{
  s_app = this;

  m_keyboardInput.animate = true;
  auto outputSize = Graphics::Core::GetOutputSize();
  m_viewport = CD3DX12_VIEWPORT(0.0F, 0.0F, static_cast<float>(outputSize.right - outputSize.left),
                                static_cast<float>(outputSize.bottom - outputSize.top));
  m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(outputSize.right - outputSize.left),
                               static_cast<LONG>(outputSize.bottom - outputSize.top));
}

D3D12Multithreading::~D3D12Multithreading()
{
  s_app = nullptr;
}

Windows::Foundation::IAsyncAction D3D12Multithreading::Startup()
{
  StartLoading();

  LoadPipeline();
  LoadAssets();
  LoadContexts();

  FinishLoading();
  co_return;
}

// Load the rendering pipeline dependencies.
void D3D12Multithreading::LoadPipeline()
{
  UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
  // Enable the debug layer (requires the Graphics Tools "optional feature").
  // NOTE: Enabling the debug layer after device creation will invalidate the active device.
  {
    com_ptr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_GRAPHICS_PPV_ARGS(debugController))))
    {
      debugController->EnableDebugLayer();

      // Enable additional debug layers.
      dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif
  auto factory = Graphics::Core::GetDXGIFactory();
  auto hardwareAdapter = Graphics::Core::GetAdapter();
  auto device = Graphics::Core::GetD3DDevice();
  auto commandQueue = Graphics::Core::GetCommandQueue();
  auto swapChain = Graphics::Core::GetSwapChain();

  D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
  check_hresult(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12)));
  s_bIsEnhancedBarriersEnabled = static_cast<bool>(options12.EnhancedBarriersSupported);

  m_frameIndex = swapChain->GetCurrentBackBufferIndex();

  // Create descriptor heaps.
  {
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = Graphics::Core::GetMaxBackBufferCount();
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    check_hresult(device->CreateDescriptorHeap(&rtvHeapDesc, IID_GRAPHICS_PPV_ARGS(m_rtvHeap)));

    // Describe and create a depth stencil view (DSV) descriptor heap.
    // Each frame has its own depth stencils (to write shadows onto)
    // and then there is one for the scene itself.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1 + Graphics::Core::GetMaxBackBufferCount() * 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    check_hresult(device->CreateDescriptorHeap(&dsvHeapDesc, IID_GRAPHICS_PPV_ARGS(m_dsvHeap)));

    // Describe and create a shader resource view (SRV) and constant
    // buffer view (CBV) descriptor heap.  Heap layout: null views,
    // object diffuse + normal textures views, frame 1's shadow buffer,
    // frame 1's 2x constant buffer, frame 2's shadow buffer, frame 2's
    // 2x constant buffers, etc...
    const UINT nullSrvCount = 2;// Null descriptors are needed for out of bounds behavior reads.
    const UINT cbvCount = Graphics::Core::GetMaxBackBufferCount() * 2;
    const UINT srvCount = _countof(SampleAssets::Textures) + (Graphics::Core::GetMaxBackBufferCount() * 1);
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    check_hresult(device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_GRAPHICS_PPV_ARGS(m_cbvSrvHeap)));
    NAME_D3D12_OBJECT(m_cbvSrvHeap);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 2;// One clamp and one wrap sampler.
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    check_hresult(device->CreateDescriptorHeap(&samplerHeapDesc, IID_GRAPHICS_PPV_ARGS(m_samplerHeap)));
    NAME_D3D12_OBJECT(m_samplerHeap);

    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  check_hresult(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(m_commandAllocator)));
}

// Load the sample assets.
void D3D12Multithreading::LoadAssets()
{
  auto device = Graphics::Core::GetD3DDevice();
  auto swapChain = Graphics::Core::GetSwapChain();
  auto commandQueue = Graphics::Core::GetCommandQueue();

  // Create the root signature.
  {
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
      featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_DESCRIPTOR_RANGE1 ranges[4];// Perfomance TIP: Order from most frequent to least frequent.
    ranges[0].Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0,
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);// 2 frequently changed diffuse + normal textures - using registers t1 and t2.
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0,
                   D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);  // 1 frequently changed constant buffer.
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);    // 1 infrequently changed shadow texture - starting in register t0.
    ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);// 2 static samplers.

    CD3DX12_ROOT_PARAMETER1 rootParameters[4];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
                               D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    com_ptr<ID3DBlob> signature;
    com_ptr<ID3DBlob> error;
    check_hresult(
        D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, signature.put(), error.put()));
    check_hresult(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                IID_GRAPHICS_PPV_ARGS(m_rootSignature)));
    NAME_D3D12_OBJECT(m_rootSignature);
  }

  // Create the pipeline state, which includes loading shaders.
  {
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
    inputLayoutDesc.pInputElementDescs = SampleAssets::StandardVertexDescription;
    inputLayoutDesc.NumElements = _countof(SampleAssets::StandardVertexDescription);

    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    // Describe and create the PSO for rendering the scene.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.pRootSignature = m_rootSignature.get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_pSpaceObjectsVS, sizeof(g_pSpaceObjectsVS));
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_pSpaceObjectsPS, sizeof(g_pSpaceObjectsPS));
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Graphics::Core::GetBackBufferFormat();
    psoDesc.DSVFormat = Graphics::Core::GetDepthBufferFormat();
    psoDesc.SampleDesc.Count = 1;

    check_hresult(device->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(m_pipelineState)));
    NAME_D3D12_OBJECT(m_pipelineState);

    // Alter the description and create the PSO for rendering
    // the shadow map.  The shadow map does not use a pixel
    // shader or render targets.
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.NumRenderTargets = 0;

    check_hresult(device->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(m_pipelineStateShadowMap)));
    NAME_D3D12_OBJECT(m_pipelineStateShadowMap);
  }

  // Create temporary command list for initial GPU setup.
  com_ptr<ID3D12GraphicsCommandList8> commandList;
  check_hresult(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), m_pipelineState.get(),
                                            IID_GRAPHICS_PPV_ARGS(commandList)));

  // Create render target views (RTVs).
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < Graphics::Core::GetBackBufferCount(); i++)
  {
    check_hresult(swapChain->GetBuffer(i, IID_GRAPHICS_PPV_ARGS(m_renderTargets[i])));
    device->CreateRenderTargetView(m_renderTargets[i].get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, m_rtvDescriptorSize);

    NAME_D3D12_OBJECT_INDEXED(m_renderTargets, i);
  }

  // Create the depth stencil.
  {
    CD3DX12_RESOURCE_DESC shadowTextureDesc(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, static_cast<UINT>(m_viewport.Width),
                                            static_cast<UINT>(m_viewport.Height), 1, 1, DXGI_FORMAT_D32_FLOAT, 1, 0,
                                            D3D12_TEXTURE_LAYOUT_UNKNOWN,
                                            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

    D3D12_CLEAR_VALUE clearValue;// Performance tip: Tell the runtime at resource creation the desired clear value.
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    if (s_bIsEnhancedBarriersEnabled)
    {
      auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      auto shadowTextureDesc1 = CD3DX12_RESOURCE_DESC1(shadowTextureDesc);
      check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &shadowTextureDesc1,
                                                       D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE, &clearValue, nullptr, 0, nullptr,
                                                       IID_GRAPHICS_PPV_ARGS(m_depthStencil)));
    }
    else
    {
      auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &shadowTextureDesc,
                                                      D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
                                                      IID_GRAPHICS_PPV_ARGS(m_depthStencil)));
    }

    NAME_D3D12_OBJECT(m_depthStencil);

    // Create the depth stencil view.
    device->CreateDepthStencilView(m_depthStencil.get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
  }

  // Load scene assets.
  auto pAssetData = BinaryFile::ReadFile(SampleAssets::DataFileName);
  ASSERT(pAssetData.size() != 0);

  // Create the vertex buffer.
  {
    if (s_bIsEnhancedBarriersEnabled)
    {
      auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      auto vertexBufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(SampleAssets::VertexDataSize);
      check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc,
                                                       D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr,
                                                       IID_GRAPHICS_PPV_ARGS(m_vertexBuffer)));
    }
    else
    {
      auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize);
      check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc,
                                                      D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                      IID_GRAPHICS_PPV_ARGS(m_vertexBuffer)));
    }

    NAME_D3D12_OBJECT(m_vertexBuffer);

    {
      if (s_bIsEnhancedBarriersEnabled)
      {
        auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto vertexBufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(SampleAssets::VertexDataSize);
        check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc,
                                                         D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr,
                                                         IID_GRAPHICS_PPV_ARGS(m_vertexBufferUpload)));
      }
      else
      {
        auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize);
        check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc,
                                                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                        IID_GRAPHICS_PPV_ARGS(m_vertexBufferUpload)));
      }

      // Copy data to the upload heap and then schedule a copy
      // from the upload heap to the vertex buffer.
      D3D12_SUBRESOURCE_DATA vertexData = {};
      vertexData.pData = pAssetData.data() + SampleAssets::VertexDataOffset;
      vertexData.RowPitch = SampleAssets::VertexDataSize;
      vertexData.SlicePitch = vertexData.RowPitch;

      PIXBeginEvent(commandList.get(), 0, L"Copy vertex buffer data to default resource...");

      UpdateSubresources<1>(commandList.get(), m_vertexBuffer.get(), m_vertexBufferUpload.get(), 0, 0, 1, &vertexData);

      if (s_bIsEnhancedBarriersEnabled)
      {
        D3D12_BUFFER_BARRIER VertexBufBarriers[] = {CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_COPY,           // SyncBefore
                                                                           D3D12_BARRIER_SYNC_VERTEX_SHADING, // SyncAfter
                                                                           D3D12_BARRIER_ACCESS_COPY_DEST,    // AccessBefore
                                                                           D3D12_BARRIER_ACCESS_VERTEX_BUFFER,// AccessAfter
                                                                           m_vertexBuffer.get())};
        D3D12_BARRIER_GROUP VertexBufBarrierGroups[] = {CD3DX12_BARRIER_GROUP(_countof(VertexBufBarriers), VertexBufBarriers)};
        commandList->Barrier(_countof(VertexBufBarrierGroups), VertexBufBarrierGroups);
      }
      else
      {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        commandList->ResourceBarrier(1, &barrier);
      }
      PIXEndEvent(commandList.get());
    }

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = SampleAssets::VertexDataSize;
    m_vertexBufferView.StrideInBytes = SampleAssets::StandardVertexStride;
  }

  // Create the index buffer.
  {
    if (s_bIsEnhancedBarriersEnabled)
    {
      auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      auto indexBufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(SampleAssets::IndexDataSize);
      check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc,
                                                       D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr,
                                                       IID_GRAPHICS_PPV_ARGS(m_indexBuffer)));
    }
    else
    {
      auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize);
      check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc,
                                                      D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                      IID_GRAPHICS_PPV_ARGS(m_indexBuffer)));
    }

    NAME_D3D12_OBJECT(m_indexBuffer);

    {
      if (s_bIsEnhancedBarriersEnabled)
      {
        auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto indexBufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(SampleAssets::IndexDataSize);
        check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc,
                                                         D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr,
                                                         IID_GRAPHICS_PPV_ARGS(m_indexBufferUpload)));
      }
      else
      {
        auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize);
        check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc,
                                                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                        IID_GRAPHICS_PPV_ARGS(m_indexBufferUpload)));
      }

      // Copy data to the upload heap and then schedule a copy
      // from the upload heap to the index buffer.
      D3D12_SUBRESOURCE_DATA indexData = {};
      indexData.pData = pAssetData.data() + SampleAssets::IndexDataOffset;
      indexData.RowPitch = SampleAssets::IndexDataSize;
      indexData.SlicePitch = indexData.RowPitch;

      PIXBeginEvent(commandList.get(), 0, L"Copy index buffer data to default resource...");

      UpdateSubresources<1>(commandList.get(), m_indexBuffer.get(), m_indexBufferUpload.get(), 0, 0, 1, &indexData);
      if (s_bIsEnhancedBarriersEnabled)
      {
        D3D12_BUFFER_BARRIER BufBarriers[] = {CD3DX12_BUFFER_BARRIER(D3D12_BARRIER_SYNC_COPY,          // SyncBefore
                                                                     D3D12_BARRIER_SYNC_INDEX_INPUT,   // SyncAfter
                                                                     D3D12_BARRIER_ACCESS_COPY_DEST,   // AccessBefore
                                                                     D3D12_BARRIER_ACCESS_INDEX_BUFFER,// AccessAfter
                                                                     m_indexBuffer.get())};
        D3D12_BARRIER_GROUP BufBarrierGroups[] = {CD3DX12_BARRIER_GROUP(_countof(BufBarriers), BufBarriers)};
        commandList->Barrier(_countof(BufBarrierGroups), BufBarrierGroups);
      }
      else
      {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                            D3D12_RESOURCE_STATE_INDEX_BUFFER);
        commandList->ResourceBarrier(1, &barrier);
      }

      PIXEndEvent(commandList.get());
    }

    // Initialize the index buffer view.
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = SampleAssets::IndexDataSize;
    m_indexBufferView.Format = SampleAssets::StandardIndexFormat;
  }

  // Create shader resources.
  {
    // Get the CBV SRV descriptor size for the current device.
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Get a handle to the start of the descriptor heap.
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());

    {
      // Describe and create 2 null SRVs. Null descriptors are needed in order
      // to achieve the effect of an "unbound" resource.
      D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
      nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      nullSrvDesc.Texture2D.MipLevels = 1;
      nullSrvDesc.Texture2D.MostDetailedMip = 0;
      nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

      device->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
      cbvSrvHandle.Offset(cbvSrvDescriptorSize);

      device->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvHandle);
      cbvSrvHandle.Offset(cbvSrvDescriptorSize);
    }

    // Create each texture and SRV descriptor.
    const UINT srvCount = _countof(SampleAssets::Textures);
    PIXBeginEvent(commandList.get(), 0, L"Copy diffuse and normal texture data to default resources...");
    for (UINT i = 0; i < srvCount; i++)
    {
      // Describe and create a Texture2D.
      const SampleAssets::TextureResource& tex = SampleAssets::Textures[i];
      CD3DX12_RESOURCE_DESC texDesc(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, tex.Width, tex.Height, 1,
                                    static_cast<UINT16>(tex.MipLevels), tex.Format, 1, 0, D3D12_TEXTURE_LAYOUT_UNKNOWN,
                                    D3D12_RESOURCE_FLAG_NONE);

      if (s_bIsEnhancedBarriersEnabled)
      {
        auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto texDesc1 = CD3DX12_RESOURCE_DESC1(texDesc);
        check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &texDesc1,
                                                         D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr, nullptr, 0, nullptr,
                                                         IID_GRAPHICS_PPV_ARGS(m_textures[i])));
      }
      else
      {
        auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &texDesc,
                                                        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                        IID_GRAPHICS_PPV_ARGS(m_textures[i])));
      }

      NAME_D3D12_OBJECT_INDEXED(m_textures, i);

      {
        const UINT subresourceCount = texDesc.DepthOrArraySize * texDesc.MipLevels;
        UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textures[i].get(), 0, subresourceCount);

        if (s_bIsEnhancedBarriersEnabled)
        {
          auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
          auto uploadBufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(uploadBufferSize);
          check_hresult(device->CreateCommittedResource3(&heapproperties, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
                                                           D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr,
                                                           IID_GRAPHICS_PPV_ARGS(m_textureUploads[i])));
        }
        else
        {
          auto heapproperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
          auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
          check_hresult(device->CreateCommittedResource(&heapproperties, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
                                                          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                          IID_GRAPHICS_PPV_ARGS(m_textureUploads[i])));
        }

        // Copy data to the intermediate upload heap and then schedule a copy
        // from the upload heap to the Texture2D.
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = pAssetData.data() + tex.Data->Offset;
        textureData.RowPitch = tex.Data->Pitch;
        textureData.SlicePitch = tex.Data->Size;

        UpdateSubresources(commandList.get(), m_textures[i].get(), m_textureUploads[i].get(), 0, 0, subresourceCount,
                           &textureData);

        if (s_bIsEnhancedBarriersEnabled)
        {
          D3D12_TEXTURE_BARRIER TexturesBarriers[] = {
              CD3DX12_TEXTURE_BARRIER(D3D12_BARRIER_SYNC_COPY,             // SyncBefore
                                      D3D12_BARRIER_SYNC_PIXEL_SHADING,    // SyncAfter
                                      D3D12_BARRIER_ACCESS_COPY_DEST,      // AccessBefore
                                      D3D12_BARRIER_ACCESS_SHADER_RESOURCE,// AccessAfter
                                      D3D12_BARRIER_LAYOUT_COPY_DEST,      // LayoutBefore
                                      D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,// LayoutAfter
                                      m_textures[i].get(),
                                      CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),// All subresources
                                      D3D12_TEXTURE_BARRIER_FLAG_NONE)};
          D3D12_BARRIER_GROUP TextureBarrierGroups[] = {CD3DX12_BARRIER_GROUP(_countof(TexturesBarriers), TexturesBarriers)};
          commandList->Barrier(_countof(TextureBarrierGroups), TextureBarrierGroups);
        }
        else
        {
          auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_textures[i].get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
          commandList->ResourceBarrier(1, &barrier);
        }
      }

      // Describe and create an SRV.
      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Format = tex.Format;
      srvDesc.Texture2D.MipLevels = tex.MipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
      device->CreateShaderResourceView(m_textures[i].get(), &srvDesc, cbvSrvHandle);

      // Move to the next descriptor slot.
      cbvSrvHandle.Offset(cbvSrvDescriptorSize);
    }
    PIXEndEvent(commandList.get());
  }

  // Create the samplers.
  {
    // Get the sampler descriptor size for the current device.
    const UINT samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    // Get a handle to the start of the descriptor heap.
    CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(m_samplerHeap->GetCPUDescriptorHandleForHeapStart());

    // Describe and create the wrapping sampler, which is used for
    // sampling diffuse/normal maps.
    D3D12_SAMPLER_DESC wrapSamplerDesc = {};
    wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    wrapSamplerDesc.MinLOD = 0;
    wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    wrapSamplerDesc.MipLODBias = 0.0f;
    wrapSamplerDesc.MaxAnisotropy = 1;
    wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] =
        wrapSamplerDesc.BorderColor[3] = 0;
    device->CreateSampler(&wrapSamplerDesc, samplerHandle);

    // Move the handle to the next slot in the descriptor heap.
    samplerHandle.Offset(samplerDescriptorSize);

    // Describe and create the point clamping sampler, which is
    // used for the shadow map.
    D3D12_SAMPLER_DESC clampSamplerDesc = {};
    clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    clampSamplerDesc.MipLODBias = 0.0f;
    clampSamplerDesc.MaxAnisotropy = 1;
    clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    clampSamplerDesc.BorderColor[0] = clampSamplerDesc.BorderColor[1] = clampSamplerDesc.BorderColor[2] =
        clampSamplerDesc.BorderColor[3] = 0;
    clampSamplerDesc.MinLOD = 0;
    clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    device->CreateSampler(&clampSamplerDesc, samplerHandle);
  }

  // Create lights.
  for (int i = 0; i < NumLights; i++)
  {
    // Set up each of the light positions and directions (they all start
    // in the same place).
    m_lights[i].position = {0.0f, 15.0f, -30.0f, 1.0f};
    m_lights[i].direction = {0.0, 0.0f, 1.0f, 0.0f};
    m_lights[i].falloff = {800.0f, 1.0f, 0.0f, 1.0f};
    m_lights[i].color = {0.7f, 0.7f, 0.7f, 1.0f};

    XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
    XMVECTOR at = XMVectorAdd(eye, XMLoadFloat4(&m_lights[i].direction));
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_lightCameras[i].Set(eye, at, up);
  }

  // Close the command list and use it to execute the initial GPU setup.
  check_hresult(commandList->Close());
  ID3D12CommandList* ppCommandLists[] = {commandList.get()};
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // Create frame resources.
  for (int i = 0; i < Graphics::Core::GetMaxBackBufferCount(); i++)
  {
    m_frameResources[i] = new FrameResource(device, m_pipelineState.get(), m_pipelineStateShadowMap.get(),
                                            m_dsvHeap.get(), m_cbvSrvHeap.get(), &m_viewport, i);
    m_frameResources[i]->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
  }
  m_currentFrameResourceIndex = 0;
  m_pCurrentFrameResource = m_frameResources[m_currentFrameResourceIndex];

  // Create synchronization objects and wait until assets have been uploaded to the GPU.
  {
    check_hresult(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(m_fence)));
    m_fenceValue++;

    // Create an event handle to use for frame synchronization.
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) { check_hresult(HRESULT_FROM_WIN32(GetLastError())); }

    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.

    // Signal and increment the fence value.
    const UINT64 fenceToWaitFor = m_fenceValue;
    check_hresult(commandQueue->Signal(m_fence.get(), fenceToWaitFor));
    m_fenceValue++;

    // Wait until the fence is completed.
    check_hresult(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }
}

// Initialize threads and events.
void D3D12Multithreading::LoadContexts()
{
#if !SINGLETHREADED
  struct threadwrapper
  {
    static unsigned int WINAPI thunk(LPVOID lpParameter)
    {
      ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
      D3D12Multithreading::Get()->WorkerThread(parameter->threadIndex);
      return 0;
    }
  };

  for (int i = 0; i < NumContexts; i++)
  {
    m_workerBeginRenderFrame[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_workerFinishedRenderFrame[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_workerFinishShadowPass[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_threadParameters[i].threadIndex = i;

    m_threadHandles[i] = reinterpret_cast<HANDLE>(
        _beginthreadex(nullptr, 0, threadwrapper::thunk, reinterpret_cast<LPVOID>(&m_threadParameters[i]), 0, nullptr));

    assert(m_workerBeginRenderFrame[i] != NULL);
    assert(m_workerFinishedRenderFrame[i] != NULL);
    assert(m_threadHandles[i] != NULL);
  }
#endif
}

// Update frame-based values.
void D3D12Multithreading::Update(float _deltaT)
{
  auto commandQueue = Graphics::Core::GetCommandQueue();

  if (IsLoading()) { return; }

  PIXSetMarker(commandQueue, 0, L"Getting last completed fence.");

  // Get current GPU progress against submitted workload. Resources still scheduled
  // for GPU execution cannot be modified or else undefined behavior will result.
  const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

  // Move to the next frame resource.
  m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % Graphics::Core::GetMaxBackBufferCount();
  m_pCurrentFrameResource = m_frameResources[m_currentFrameResourceIndex];

  // Make sure that this frame resource isn't still in use by the GPU.
  // If it is, wait for it to complete.
  if (m_pCurrentFrameResource->m_fenceValue > lastCompletedFence)
  {
    HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (eventHandle == nullptr) { check_hresult(HRESULT_FROM_WIN32(GetLastError())); }
    check_hresult(m_fence->SetEventOnCompletion(m_pCurrentFrameResource->m_fenceValue, eventHandle));
    WaitForSingleObject(eventHandle, INFINITE);
    CloseHandle(eventHandle);
  }

  float frameChange = 2.0f * _deltaT;

  if (m_keyboardInput.leftArrowPressed) m_camera.RotateYaw(-frameChange);
  if (m_keyboardInput.rightArrowPressed) m_camera.RotateYaw(frameChange);
  if (m_keyboardInput.upArrowPressed) m_camera.RotatePitch(frameChange);
  if (m_keyboardInput.downArrowPressed) m_camera.RotatePitch(-frameChange);

  if (m_keyboardInput.animate)
  {
    for (int i = 0; i < NumLights; i++)
    {
      float direction = frameChange * powf(-1.0f, static_cast<float>(i));
      XMStoreFloat4(&m_lights[i].position, XMVector4Transform(XMLoadFloat4(&m_lights[i].position), XMMatrixRotationY(direction)));

      XMVECTOR eye = XMLoadFloat4(&m_lights[i].position);
      XMVECTOR at = XMVectorSet(0.0f, 8.0f, 0.0f, 0.0f);
      XMStoreFloat4(&m_lights[i].direction, XMVector3Normalize(XMVectorSubtract(at, eye)));
      XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
      m_lightCameras[i].Set(eye, at, up);

      auto outputSize = Graphics::Core::GetOutputSize();
      float width = outputSize.right - outputSize.left;
      float height = outputSize.bottom - outputSize.top;
      m_lightCameras[i].Get3DViewProjMatrices(&m_lights[i].view, &m_lights[i].projection, 90.0f, width, height);
    }
  }

  m_pCurrentFrameResource->WriteConstantBuffers(&m_viewport, &m_camera, m_lightCameras, m_lights);
}

// Render the scene.
void D3D12Multithreading::RenderScene()
{
  auto swapChain = Graphics::Core::GetSwapChain();
  auto device = Graphics::Core::GetD3DDevice();
  auto commandQueue = Graphics::Core::GetCommandQueue();

  if (IsLoading()) { return; }

  BeginFrame();

  for (int i = 0; i < NumContexts; i++)
  {
    SetEvent(m_workerBeginRenderFrame[i]);// Tell each worker to start drawing.
  }

  MidFrame();
  EndFrame();

  WaitForMultipleObjects(NumContexts, m_workerFinishShadowPass, TRUE, INFINITE);

  // You can execute command lists on any thread. Depending on the work
  // load, apps can choose between using ExecuteCommandLists on one thread
  // vs ExecuteCommandList from multiple threads.
  commandQueue->ExecuteCommandLists(NumContexts + 2, m_pCurrentFrameResource->m_batchSubmit);// Submit PRE, MID and shadows.

  WaitForMultipleObjects(NumContexts, m_workerFinishedRenderFrame, TRUE, INFINITE);

  // Submit remaining command lists.
  commandQueue->ExecuteCommandLists(_countof(m_pCurrentFrameResource->m_batchSubmit) - NumContexts - 2,
                                      m_pCurrentFrameResource->m_batchSubmit + NumContexts + 2);

  // Present and update the frame index for the next frame.
  PIXBeginEvent(commandQueue, 0, L"Presenting to screen");
  check_hresult(swapChain->Present(1, 0));
  PIXEndEvent(commandQueue);
  m_frameIndex = swapChain->GetCurrentBackBufferIndex();

  // Signal and increment the fence value.
  m_pCurrentFrameResource->m_fenceValue = m_fenceValue;
  check_hresult(commandQueue->Signal(m_fence.get(), m_fenceValue));
  m_fenceValue++;
}

// Release sample's D3D objects.
void D3D12Multithreading::ReleaseD3DResources()
{
  m_fence = nullptr;
  ResetComPtrArray(&m_renderTargets);
}

// Tears down D3D resources and reinitializes them.
void D3D12Multithreading::RestoreD3DResources()
{
  // Give GPU a chance to finish its execution in progress.
  WaitForGpu();
  ReleaseD3DResources();
  Startup();
}

// Wait for pending GPU work to complete.
void D3D12Multithreading::WaitForGpu()
{
  auto commandQueue = Graphics::Core::GetCommandQueue();

  // Schedule a Signal command in the queue.
  check_hresult(commandQueue->Signal(m_fence.get(), m_fenceValue));

  // Wait until the fence has been processed.
  check_hresult(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
  WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
}

void D3D12Multithreading::Shutdown()
{
  auto commandQueue = Graphics::Core::GetCommandQueue();

  // Ensure that the GPU is no longer referencing resources that are about to be
  // cleaned up by the destructor.
  {
    const UINT64 fence = m_fenceValue;
    const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

    // Signal and increment the fence value.
    check_hresult(commandQueue->Signal(m_fence.get(), m_fenceValue));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (lastCompletedFence < fence)
    {
      check_hresult(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
      WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    CloseHandle(m_fenceEvent);
  }

  // Close thread events and thread handles.
  for (int i = 0; i < NumContexts; i++)
  {
    CloseHandle(m_workerBeginRenderFrame[i]);
    CloseHandle(m_workerFinishShadowPass[i]);
    CloseHandle(m_workerFinishedRenderFrame[i]);
    CloseHandle(m_threadHandles[i]);
  }

  for (int i = 0; i < _countof(m_frameResources); i++) { delete m_frameResources[i]; }
}

void D3D12Multithreading::OnKeyDown(UINT8 key)
{
  switch (key)
  {
    case VK_LEFT:
      m_keyboardInput.leftArrowPressed = true;
      break;
    case VK_RIGHT:
      m_keyboardInput.rightArrowPressed = true;
      break;
    case VK_UP:
      m_keyboardInput.upArrowPressed = true;
      break;
    case VK_DOWN:
      m_keyboardInput.downArrowPressed = true;
      break;
    case VK_SPACE:
      m_keyboardInput.animate = !m_keyboardInput.animate;
      break;
  }
}

void D3D12Multithreading::OnKeyUp(UINT8 key)
{
  switch (key)
  {
    case VK_LEFT:
      m_keyboardInput.leftArrowPressed = false;
      break;
    case VK_RIGHT:
      m_keyboardInput.rightArrowPressed = false;
      break;
    case VK_UP:
      m_keyboardInput.upArrowPressed = false;
      break;
    case VK_DOWN:
      m_keyboardInput.downArrowPressed = false;
      break;
  }
}

// Assemble the CommandListPre command list.
void D3D12Multithreading::BeginFrame()
{
  m_pCurrentFrameResource->Init();

  if (s_bIsEnhancedBarriersEnabled)
  {
    D3D12_TEXTURE_BARRIER BeginFrameBarriers[] = {
        // Using SYNC_NONE and ACCESS_NO_ACCESS with Enhanced Barrier to avoid unnecessary sync/flush.
        // Using them explicitly tells the GPU it is okay to immediately transition
        // the layout without waiting for preceding work to complete.
        // In this case, the legacy barrier would flush and finish any preceding work that may potentially be reading from the RT
        // resource (in this case there is none)
        CD3DX12_TEXTURE_BARRIER(D3D12_BARRIER_SYNC_NONE,           // SyncBefore
                                D3D12_BARRIER_SYNC_RENDER_TARGET,  // SyncAfter
                                D3D12_BARRIER_ACCESS_NO_ACCESS,    // AccessBefore
                                D3D12_BARRIER_ACCESS_RENDER_TARGET,// AccessAfter
                                D3D12_BARRIER_LAYOUT_PRESENT,      // LayoutBefore
                                D3D12_BARRIER_LAYOUT_RENDER_TARGET,// LayoutAfter
                                m_renderTargets[m_frameIndex].get(),
                                CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),// All subresources
                                D3D12_TEXTURE_BARRIER_FLAG_NONE)};

    D3D12_BARRIER_GROUP BeginFrameBarriersGroups[] = {CD3DX12_BARRIER_GROUP(_countof(BeginFrameBarriers), BeginFrameBarriers)};

    m_pCurrentFrameResource->m_commandLists[CommandListPre]->Barrier(_countof(BeginFrameBarriersGroups),
                                                                     BeginFrameBarriersGroups);
  }
  else
  {
    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].get(), D3D12_RESOURCE_STATE_PRESENT,
                                                        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_pCurrentFrameResource->m_commandLists[CommandListPre]->ResourceBarrier(1, &barrier);
  }

  // Clear the render target and depth stencil.
  const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
  m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
  m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
                                                                                 D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

  check_hresult(m_pCurrentFrameResource->m_commandLists[CommandListPre]->Close());
}

// Assemble the CommandListMid command list.
void D3D12Multithreading::MidFrame()
{
  // Transition our shadow map from the shadow pass to readable in the scene pass.
  m_pCurrentFrameResource->SwapBarriers();

  check_hresult(m_pCurrentFrameResource->m_commandLists[CommandListMid]->Close());
}

// Assemble the CommandListPost command list.
void D3D12Multithreading::EndFrame()
{
  m_pCurrentFrameResource->Finish();

  if (s_bIsEnhancedBarriersEnabled)
  {
    D3D12_TEXTURE_BARRIER EndFrameBarriers[] = {
        // Using SYNC_NONE and ACCESS_NO_ACCESS with Enhanced Barrier means subsequent
        // commands are unblocked without having to wait for the barrier to complete.
        CD3DX12_TEXTURE_BARRIER(D3D12_BARRIER_SYNC_RENDER_TARGET,  // SyncBefore
                                D3D12_BARRIER_SYNC_NONE,           // SyncAfter
                                D3D12_BARRIER_ACCESS_RENDER_TARGET,// AccessBefore
                                D3D12_BARRIER_ACCESS_NO_ACCESS,    // AccessAfter
                                D3D12_BARRIER_LAYOUT_RENDER_TARGET,// LayoutBefore
                                D3D12_BARRIER_LAYOUT_PRESENT,      // LayoutAfter
                                m_renderTargets[m_frameIndex].get(),
                                CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff),// All subresources
                                D3D12_TEXTURE_BARRIER_FLAG_NONE)};
    D3D12_BARRIER_GROUP EndFrameBarrierGroups[] = {CD3DX12_BARRIER_GROUP(_countof(EndFrameBarriers), EndFrameBarriers)};
    m_pCurrentFrameResource->m_commandLists[CommandListPost]->Barrier(_countof(EndFrameBarrierGroups), EndFrameBarrierGroups);
  }
  else
  {
    // Indicate that the back buffer will now be used to present.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                        D3D12_RESOURCE_STATE_PRESENT);
    m_pCurrentFrameResource->m_commandLists[CommandListPost]->ResourceBarrier(1, &barrier);
  }

  check_hresult(m_pCurrentFrameResource->m_commandLists[CommandListPost]->Close());
}

// Worker thread body. workerIndex is an integer from 0 to NumContexts
// describing the worker's thread index.
void D3D12Multithreading::WorkerThread(int threadIndex)
{
  auto device = Graphics::Core::GetD3DDevice();

  assert(threadIndex >= 0);
  assert(threadIndex < NumContexts);

  while (threadIndex >= 0 && threadIndex < NumContexts)
  {
    // Wait for main thread to tell us to draw.

    WaitForSingleObject(m_workerBeginRenderFrame[threadIndex], INFINITE);

    ID3D12GraphicsCommandList* pShadowCommandList = m_pCurrentFrameResource->m_shadowCommandLists[threadIndex].get();
    ID3D12GraphicsCommandList* pSceneCommandList = m_pCurrentFrameResource->m_sceneCommandLists[threadIndex].get();

    //
    // Shadow pass
    //

    // Populate the command list.
    SetCommonPipelineState(pShadowCommandList);
    m_pCurrentFrameResource->Bind(pShadowCommandList, FALSE, nullptr, nullptr);// No need to pass RTV or DSV descriptor heap.

    // Set null SRVs for the diffuse/normal textures.
    pShadowCommandList->SetGraphicsRootDescriptorTable(0, m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

    // Distribute objects over threads by drawing only 1/NumContexts
    // objects per worker (i.e. every object such that objectnum %
    // NumContexts == threadIndex).
    PIXBeginEvent(pShadowCommandList, 0, L"Worker drawing shadow pass...");

    for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
    {
      SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[j];

      pShadowCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
    }

    PIXEndEvent(pShadowCommandList);

    check_hresult(pShadowCommandList->Close());

    // Submit shadow pass.
    SetEvent(m_workerFinishShadowPass[threadIndex]);

    //
    // Scene pass
    //

    // Populate the command list.  These can only be sent after the shadow
    // passes for this frame have been submitted.
    SetCommonPipelineState(pSceneCommandList);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_pCurrentFrameResource->Bind(pSceneCommandList, TRUE, &rtvHandle, &dsvHandle);

    PIXBeginEvent(pSceneCommandList, 0, L"Worker drawing scene pass...");

    D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const UINT nullSrvCount = 2;
    for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
    {
      SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[j];

      // Set the diffuse and normal textures for the current object.
      CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, nullSrvCount + drawArgs.DiffuseTextureIndex,
                                                 cbvSrvDescriptorSize);
      pSceneCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

      pSceneCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
    }

    PIXEndEvent(pSceneCommandList);
    check_hresult(pSceneCommandList->Close());

    // Tell main thread that we are done.
    SetEvent(m_workerFinishedRenderFrame[threadIndex]);
  }
}

void D3D12Multithreading::SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList)
{
  pCommandList->SetGraphicsRootSignature(m_rootSignature.get());

  ID3D12DescriptorHeap* ppHeaps[] = {m_cbvSrvHeap.get(), m_samplerHeap.get()};
  pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  pCommandList->RSSetViewports(1, &m_viewport);
  pCommandList->RSSetScissorRects(1, &m_scissorRect);
  pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
  pCommandList->IASetIndexBuffer(&m_indexBufferView);
  pCommandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
  pCommandList->OMSetStencilRef(0);

  // Render targets and depth stencil are set elsewhere because the
  // depth stencil depends on the frame resource being used.

  // Constant buffers are set elsewhere because they depend on the
  // frame resource being used.

  // SRVs are set elsewhere because they change based on the object
  // being drawn.
}
