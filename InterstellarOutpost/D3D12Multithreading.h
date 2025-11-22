#pragma once

#include "Camera.h"
#include "DXSample.h"
#include "SquidRoom.h"
#include "StepTimer.h"

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

class D3D12Multithreading : public DXSample
{
public:
  D3D12Multithreading(UINT width, UINT height, std::wstring name);
  virtual ~D3D12Multithreading();

  static D3D12Multithreading *Get() { return s_app; }

  static bool IsEnhancedBarriersEnabled() { return s_bIsEnhancedBarriersEnabled; }

  virtual void OnInit();
  virtual void OnUpdate();
  virtual void OnRender();
  virtual void OnDestroy();
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
  com_ptr<IDXGISwapChain3> m_swapChain;
  com_ptr<ID3D12Device10> m_device;
  com_ptr<ID3D12Resource> m_renderTargets[FrameCount];
  com_ptr<ID3D12Resource> m_depthStencil;
  com_ptr<ID3D12CommandAllocator> m_commandAllocator;
  com_ptr<ID3D12CommandQueue> m_commandQueue;
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
  StepTimer m_timer;
  StepTimer m_cpuTimer;
  int m_titleCount;
  double m_cpuTime;

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
  FrameResource *m_frameResources[FrameCount];
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
