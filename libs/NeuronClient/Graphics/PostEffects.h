
#pragma once

#include "pch.h"
#include "EngineTuning.h"

class ComputeContext;

namespace PostEffects
{
  extern BoolVar EnableHDR;// Turn on tone mapping features

  // Tone mapping parameters
  extern ExpVar Exposure;         // Brightness scaler when adapative exposure is disabled
  extern BoolVar EnableAdaptation;// Automatically adjust brightness based on perceived luminance

  // Adapation parameters
  extern ExpVar MinExposure;
  extern ExpVar MaxExposure;
  extern NumVar TargetLuminance;
  extern NumVar AdaptationRate;

  // Bloom parameters
  extern BoolVar BloomEnable;
  extern NumVar BloomThreshold;
  extern NumVar BloomStrength;

  void Initialize(void);
  void Shutdown(void);
  void Render(void);

  // Copy the contents of the post effects buffer onto the main scene buffer
  void CopyBackPostBuffer(ComputeContext &Context);
}// namespace PostEffects
