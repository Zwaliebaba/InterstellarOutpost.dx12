
#pragma once

namespace Neuron::Math
{
  class Camera;
}
class GraphicsContext;
class ComputeContext;
class BoolVar;

namespace SSAO
{
  void Initialize(void);
  void Shutdown(void);
  void Render(GraphicsContext &Context, const float *ProjMat, float NearClipDist, float FarClipDist);
  void Render(GraphicsContext &Context, const Math::Camera &camera);
  void LinearizeZ(ComputeContext &Context, const Math::Camera &camera, uint32_t FrameIndex);

  extern BoolVar Enable;
  extern BoolVar DebugDraw;
  extern BoolVar AsyncCompute;
  extern BoolVar ComputeLinearZ;
}// namespace SSAO
