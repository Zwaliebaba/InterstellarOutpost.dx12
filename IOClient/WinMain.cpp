#include "pch.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#if defined(_DEBUG)
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  constexpr Windows::Foundation::Size size = {1024, 768};
  Neuron::Client::Core::Startup(L"Interstellar Outpost", size, hInstance, nShowCmd);

  return 0;
}
