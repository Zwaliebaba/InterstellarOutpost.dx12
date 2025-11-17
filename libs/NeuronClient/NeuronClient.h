#pragma once

#include "NeuronCore.h"

#include "DeviceNotify.h"
#include "Graphics/DirectXHelper.h"
#include "Graphics/VectorMath.h"

#include "EngineProfiling.h"
#include "EngineTuning.h"
#include "Utility.h"

#include "GameMain.h"

namespace Neuron::Client
{
  class Core
  {
  public:
    static int Startup(const wchar_t *_gameName, Windows::Foundation::Size _size, HINSTANCE _hInstance, int nCmdShow);
    static void StartGame(const com_ptr<GameMain> &&_gameMain);
    static void Shutdown();
    static void OnDeviceLost();
    static void OnDeviceRestored();

    static HINSTANCE Instance() { return m_instance; }

    static HWND Window() { return m_hwnd; }

    static Windows::Foundation::Size OutputSize() { return m_outputSize; }

    static Windows::Foundation::Point OutputTopLeft();

    static void RegisterDeviceNotify(IDeviceNotify *deviceNotify)
    {
      m_deviceNotify = deviceNotify;

      // On RS4 and higher, applications that handle device removal
      // should declare themselves as being able to do so
      __if_exists(DXGIDeclareAdapterRemovalSupport)
      {
        if (deviceNotify)
          if (FAILED(DXGIDeclareAdapterRemovalSupport())) OutputDebugString(L"Warning: application failed to declare adapter removal support.\n");
      }
    }

  protected:
    inline static IDeviceNotify *m_deviceNotify{};
    inline static HINSTANCE m_instance;
    inline static HWND m_hwnd;
    inline static Windows::Foundation::Size m_outputSize;
    inline static com_ptr<GameMain> m_main;

    inline static bool sm_resourcesLoaded = {false};
  };
}// namespace Neuron::Client

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Shell32.lib")
