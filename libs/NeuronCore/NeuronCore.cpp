#include "pch.h"

#pragma comment(lib, "RuntimeObject.lib")

int Neuron::Core::Startup()
{
  if (!XMVerifyCPUSupport()) return 1;

  init_apartment();

  Timer::Core::Startup();

  wchar_t filename[MAX_PATH];
  GetModuleFileNameW(nullptr, filename, MAX_PATH);
  auto path = std::wstring(filename);
  path = path.substr(0, path.find_last_of('\\'));

  FileSys::SetHomeDirectory(path);

  return 0;
}

void Neuron::Core::Shutdown()
{
}