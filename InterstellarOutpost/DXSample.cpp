#include "pch.h"

#include "DXSample.h"

DXSample::DXSample(UINT width, UINT height, std::wstring name)
    : m_width(width), m_height(height), m_title(name)
{
  m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DXSample::~DXSample() {}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_ void DXSample::GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter,
                                                         bool requestHighPerformanceAdapter)
{
  *ppAdapter = nullptr;

  com_ptr<IDXGIAdapter1> adapter;

  com_ptr<IDXGIFactory6> factory6;
  if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
  {
    for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex,
                                                                               requestHighPerformanceAdapter == true
                                                                                   ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                                                                   : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                                                                               IID_PPV_ARGS(&adapter)));
         ++adapterIndex)
    {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create the
      // actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) { break; }
    }
  }

  if (adapter.get() == nullptr)
  {
    for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, adapter.put())); ++adapterIndex)
    {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      {
        // Don't select the Basic Render Driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create the
      // actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) { break; }
    }
  }

  *ppAdapter = adapter.detach();
}

// Helper function for setting the window's title text.
void DXSample::SetCustomWindowText(LPCWSTR text)
{
  std::wstring windowText = m_title + L": " + text;
  SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}
