#pragma once

#include "Camera.h"
#include "SquidRoom.h"

class FrameResource;

struct LightState
{
  XMFLOAT4 position;
  XMFLOAT4 direction;
  XMFLOAT4 color;
  XMFLOAT4 falloff;

  XMFLOAT4X4 view;
  XMFLOAT4X4 projection;
};

struct SceneConstantBuffer
{
  XMFLOAT4X4 model;
  XMFLOAT4X4 view;
  XMFLOAT4X4 projection;
  XMFLOAT4 ambientColor;
  BOOL sampleShadowMap;
  BOOL padding[3];// Must be aligned to be made up of N float4s.
  LightState lights[NumLights];
};

class D3D12Multithreading : public GameMain
{
public:
  D3D12Multithreading();
  virtual ~D3D12Multithreading();

  static D3D12Multithreading *Get() { return s_app; }

  static bool IsEnhancedBarriersEnabled() { return s_bIsEnhancedBarriersEnabled; }

  virtual Windows::Foundation::IAsyncAction Startup();
  virtual void Shutdown();
  virtual void Update(float _deltaT);
  virtual void RenderScene();
  virtual void OnKeyDown(UINT8 key);
  virtual void OnKeyUp(UINT8 key);

private:
  struct InputState
  {
    bool rightArrowPressed;
    bool leftArrowPressed;
    bool upArrowPressed;
    bool downArrowPressed;
    bool animate;
  };

  // Pipeline objects.
  CD3DX12_VIEWPORT m_viewport;
  CD3DX12_RECT m_scissorRect;
  com_ptr<ID3D12Resource> m_renderTargets[Graphics::Core::GetMaxBackBufferCount()];
  com_ptr<ID3D12Resource> m_depthStencil;
  com_ptr<ID3D12CommandAllocator> m_commandAllocator;
  com_ptr<ID3D12RootSignature> m_rootSignature;
  com_ptr<ID3D12DescriptorHeap> m_rtvHeap;
  com_ptr<ID3D12DescriptorHeap> m_dsvHeap;
  com_ptr<ID3D12DescriptorHeap> m_cbvSrvHeap;
  com_ptr<ID3D12DescriptorHeap> m_samplerHeap;
  com_ptr<ID3D12PipelineState> m_pipelineState;
  com_ptr<ID3D12PipelineState> m_pipelineStateShadowMap;

  // App resources.
  D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
  D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
  com_ptr<ID3D12Resource> m_textures[_countof(SampleAssets::Textures)];
  com_ptr<ID3D12Resource> m_textureUploads[_countof(SampleAssets::Textures)];
  com_ptr<ID3D12Resource> m_indexBuffer;
  com_ptr<ID3D12Resource> m_indexBufferUpload;
  com_ptr<ID3D12Resource> m_vertexBuffer;
  com_ptr<ID3D12Resource> m_vertexBufferUpload;
  UINT m_rtvDescriptorSize;
  InputState m_keyboardInput;
  LightState m_lights[NumLights];
  Camera m_lightCameras[NumLights];
  Camera m_camera;

  // Synchronization objects.
  HANDLE m_workerBeginRenderFrame[NumContexts];
  HANDLE m_workerFinishShadowPass[NumContexts];
  HANDLE m_workerFinishedRenderFrame[NumContexts];
  HANDLE m_threadHandles[NumContexts];
  UINT m_frameIndex;
  HANDLE m_fenceEvent;
  com_ptr<ID3D12Fence> m_fence;
  UINT64 m_fenceValue;

  // Singleton object so that worker threads can share members.
  static D3D12Multithreading *s_app;

  // Frame resources.
  FrameResource* m_frameResources[Graphics::Core::GetMaxBackBufferCount()];
  FrameResource *m_pCurrentFrameResource;
  int m_currentFrameResourceIndex;

  static bool s_bIsEnhancedBarriersEnabled;

  struct ThreadParameter
  {
    int threadIndex;
  };
  ThreadParameter m_threadParameters[NumContexts];

  void WorkerThread(int threadIndex);
  void SetCommonPipelineState(ID3D12GraphicsCommandList *pCommandList);

  void LoadPipeline();
  void LoadAssets();
  void RestoreD3DResources();
  void ReleaseD3DResources();
  void WaitForGpu();
  void LoadContexts();
  void BeginFrame();
  void MidFrame();
  void EndFrame();
};
