#include "pch.h"

#include "D3D12Multithreading.h"

int WINAPI wWinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPWSTR _cmdLine, int _iCmdShow)
{
#if defined(_DEBUG)
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  constexpr Windows::Foundation::Size size = {1280, 720};

  ClientEngine::Startup(L"Interstellar Outpost", size, _hInstance, _iCmdShow);

  auto main = winrt::make_self<D3D12Multithreading>();
  ClientEngine::StartGame(main);

  //// Main message loop
  MSG msg = {};
  while (WM_QUIT != msg.message)
  {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      auto deltaT = Timer::Core::Update();
      main->Update(deltaT);

      //Graphics::Core::Prepare();
      main->RenderScene();
      //Graphics::Core::ExecuteCommandList();
      //main->RenderCanvas();
      //Graphics::Core::Present();
    }
  }

  ClientEngine::Shutdown();
  main = nullptr;

  return static_cast<int>(msg.wParam);
}
