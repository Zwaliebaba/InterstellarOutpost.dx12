
#pragma once

class ColorBuffer;
class BoolVar;
class NumVar;
class ComputeContext;

namespace FXAA
{
  extern BoolVar Enable;
  extern BoolVar DebugDraw;
  extern NumVar ContrastThreshold;// Default = 0.20
  extern NumVar SubpixelRemoval;  // Default = 0.75

  void Initialize(void);
  void Shutdown(void);
  void Render(ComputeContext &Context, bool bUsePreComputedLuma);

}// namespace FXAA
