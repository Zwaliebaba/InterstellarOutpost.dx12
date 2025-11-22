#include "pch.h"

void CoreEngine::Startup()
{
  if (!XMVerifyCPUSupport())
    Fatal(L"CPU does not support the right technology");

  Timer::Core::Startup();

  wchar_t filename[MAX_PATH];
  GetModuleFileNameW(nullptr, filename, MAX_PATH);
  auto path = std::wstring(filename);
  path = path.substr(0, path.find_last_of('\\'));

  FileSys::SetHomeDirectory(path);
}

void CoreEngine::Shutdown()
{
}
