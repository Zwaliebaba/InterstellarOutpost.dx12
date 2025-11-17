
#pragma once

#include "EngineTuning.h"

namespace DepthOfField
{
  extern BoolVar Enable;

  void Initialize(void);
  void Shutdown(void);

  void Render(CommandContext &BaseContext, float NearClipDist, float FarClipDist);
}// namespace DepthOfField
