
#pragma once

#include <cstdint>

namespace Display
{
  void Initialize(void);
  void Shutdown(void);
  void Resize(uint32_t width, uint32_t height);
  void Present(void);
}// namespace Display

namespace Graphics
{
  extern uint32_t g_DisplayWidth;
  extern uint32_t g_DisplayHeight;
  extern bool g_bEnableHDROutput;

  // Returns the number of elapsed frames since application start
  uint64_t GetFrameCount(void);

  // The amount of time elapsed during the last completed frame.  The CPU and/or
  // GPU may be idle during parts of the frame.  The frame time measures the time
  // between calls to present each frame.
  float GetFrameTime(void);

  // The total number of frames per second
  float GetFrameRate(void);

  extern bool g_bEnableHDROutput;
}// namespace Graphics
