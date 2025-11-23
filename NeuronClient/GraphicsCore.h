#pragma once

namespace Neuron::Graphics
{
  // Controls all the DirectX device resources.
  class Core
  {
  public:
    static const unsigned int c_AllowTearing = 0x1;
    static const unsigned int c_RequireTearingSupport = 0x2;

    static void Startup(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
                        DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT, UINT backBufferCount = 2,
                        D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0, UINT flags = 0,
                        UINT adapterIDoverride = UINT_MAX);
    static void Shutdown();

    static void InitializeDXGIAdapter();
    static void SetAdapterOverride(UINT adapterID) { m_adapterIDoverride = adapterID; }
    static void CreateDeviceResources();
    static void CreateWindowSizeDependentResources();
    static void SetWindow(HWND window, int width, int height);
    static bool WindowSizeChanged(int width, int height, bool minimized);
    static void HandleDeviceLost();

    static void Prepare(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT);
    static void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
    static void ExecuteCommandList();
    static void WaitForGpu() noexcept;

    // Device Accessors.
    static RECT GetOutputSize() { return m_outputSize; }
    static bool IsWindowVisible() { return m_isWindowVisible; }
    static bool IsTearingSupported() { return m_options & c_AllowTearing; }

    // Direct3D Accessors.
    static IDXGIAdapter1* GetAdapter() { return m_adapter.get(); }
    static ID3D12Device10* GetD3DDevice() { return m_d3dDevice.get(); }
    static IDXGIFactory4* GetDXGIFactory() { return m_dxgiFactory.get(); }
    static IDXGISwapChain3* GetSwapChain() { return m_swapChain.get(); }
    static D3D_FEATURE_LEVEL GetDeviceFeatureLevel() { return m_d3dFeatureLevel; }
    static ID3D12Resource* GetRenderTarget() { return m_renderTargets[m_backBufferIndex].get(); }
    static ID3D12Resource* GetDepthStencil() { return m_depthStencil.get(); }
    static ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.get(); }
    static ID3D12CommandAllocator* GetCommandAllocator() { return m_commandAllocators[m_backBufferIndex].get(); }
    static auto* GetCommandList() { return m_commandList.get(); }
    static DXGI_FORMAT GetBackBufferFormat() { return m_backBufferFormat; }
    static DXGI_FORMAT GetDepthBufferFormat() { return m_depthBufferFormat; }
    static D3D12_VIEWPORT GetScreenViewport() { return m_screenViewport; }
    static D3D12_RECT GetScissorRect() { return m_scissorRect; }
    static UINT GetCurrentFrameIndex() { return m_backBufferIndex; }
    static UINT GetPreviousFrameIndex() { return m_backBufferIndex == 0 ? m_backBufferCount - 1 : m_backBufferIndex - 1; }
    static UINT GetBackBufferCount() { return m_backBufferCount; }
    static constexpr UINT GetMaxBackBufferCount() { return MAX_BACK_BUFFER_COUNT; }
    static unsigned int GetDeviceOptions() { return m_options; }
    static LPCWSTR GetAdapterDescription() { return m_adapterDescription.c_str(); }
    static UINT GetAdapterID() { return m_adapterID; }
    static bool IsEnhancedBarriersEnabled() { return m_isEnhancedBarriersEnabled; }

    static CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView()
    {
      return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex,
                                           m_rtvDescriptorSize);
    }
    static CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView()
    {
      return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }

  private:
    static void MoveToNextFrame();
    static void InitializeAdapter(IDXGIAdapter1** ppAdapter);

    const static size_t MAX_BACK_BUFFER_COUNT = 3;

    inline static UINT m_adapterIDoverride;
    inline static UINT m_backBufferIndex;
    inline static com_ptr<IDXGIAdapter1> m_adapter;
    inline static UINT m_adapterID = UINT_MAX;
    inline static std::wstring m_adapterDescription;

    // Direct3D objects.
    inline static com_ptr<ID3D12Device10> m_d3dDevice;
    inline static com_ptr<ID3D12CommandQueue> m_commandQueue;
    inline static com_ptr<ID3D12GraphicsCommandList10> m_commandList;
    inline static com_ptr<ID3D12CommandAllocator> m_commandAllocators[MAX_BACK_BUFFER_COUNT];

    // Swap chain objects.
    inline static com_ptr<IDXGIFactory4> m_dxgiFactory;
    inline static com_ptr<IDXGISwapChain3> m_swapChain;
    inline static com_ptr<ID3D12Resource> m_renderTargets[MAX_BACK_BUFFER_COUNT];
    inline static com_ptr<ID3D12Resource> m_depthStencil;

    // Presentation fence objects.
    inline static com_ptr<ID3D12Fence> m_fence;
    inline static UINT64 m_fenceValues[MAX_BACK_BUFFER_COUNT];
    inline static handle m_fenceEvent;

    // Direct3D rendering objects.
    inline static com_ptr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    inline static com_ptr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;
    inline static UINT m_rtvDescriptorSize;
    inline static D3D12_VIEWPORT m_screenViewport;
    inline static D3D12_RECT m_scissorRect;

    // Direct3D properties.
    inline static DXGI_FORMAT m_backBufferFormat;
    inline static DXGI_FORMAT m_depthBufferFormat;
    inline static UINT m_backBufferCount;
    inline static D3D_FEATURE_LEVEL m_d3dMinFeatureLevel;

    // Cached device properties.
    inline static HWND m_window;
    inline static D3D_FEATURE_LEVEL m_d3dFeatureLevel = D3D_FEATURE_LEVEL_12_0;
    inline static RECT m_outputSize = {0, 0, 1, 1};
    inline static bool m_isWindowVisible = true;
    inline static bool m_isEnhancedBarriersEnabled = false;

    // Core options (see flags above)
    inline static unsigned int m_options;
  };
}// namespace Neuron::Graphics
