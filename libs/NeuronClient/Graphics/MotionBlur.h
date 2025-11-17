
#pragma once

#include "EngineTuning.h"

// Forward declarations
namespace Neuron::Math
{
  class Matrix4;
  class Camera;
}// namespace Neuron::Math
class ColorBuffer;
class CommandContext;

namespace MotionBlur
{
  extern BoolVar Enable;

  void Initialize(void);
  void Shutdown(void);

  void GenerateCameraVelocityBuffer(CommandContext &Context, const Math::Camera &camera, bool UseLinearZ = true);
  void GenerateCameraVelocityBuffer(CommandContext &Context, const Math::Matrix4 &reprojectionMatrix, float nearClip, float farClip,
                                    bool UseLinearZ = true);

  // Generate motion blur only associated with the camera.  Does not handle fast-moving objects well, but
  // does not require a full screen velocity buffer.
  void RenderCameraBlur(CommandContext &Context, const Math::Camera &camera, bool UseLinearZ = true);
  void RenderCameraBlur(CommandContext &Context, const Math::Matrix4 &reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ = true);

  // Generate proper motion blur that takes into account the velocity of each pixel.  Requires a pre-generated
  // velocity buffer (R16G16_FLOAT preferred.)
  void RenderObjectBlur(CommandContext &Context, ColorBuffer &velocityBuffer);
}// namespace MotionBlur
