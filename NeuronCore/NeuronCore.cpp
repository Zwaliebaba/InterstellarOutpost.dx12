#include "pch.h"
#include "TimerCore.h"

void CoreEngine::Startup()
{
  if (!XMVerifyCPUSupport())
    Fatal(L"CPU does not support the right technology");

  Timer::Core::Startup();
}

void CoreEngine::Shutdown() {}
