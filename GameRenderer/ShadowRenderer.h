#pragma once

#include "vector3.h"

// Forward declarations — keep GameRenderer decoupled from concrete subsystem headers
class Resource;
class Renderer;
class Camera;
template <class T> class SurfaceMap2D;

namespace GameRenderer {

// ---------------------------------------------------------------------------
// Shadow pass — extracted from Entity::BeginRenderShadow / RenderShadow / EndRenderShadow
// ---------------------------------------------------------------------------

// Begins a shadow rendering pass: binds the glow texture, sets blend/depth
// state, and adjusts the projection near plane to avoid z-fighting.
// Returns the original near plane value — pass it to EndShadowPass().
double BeginShadowPass(Resource& resource, Renderer& renderer, Camera& camera);

// Restores blend/depth/cull state and resets the projection near plane.
void EndShadowPass(double savedNearPlane, Renderer& renderer, Camera& camera);

// Renders a single entity shadow quad projected onto the terrain at |pos|
// with the given |size|.  |heightMap| is used to sample ground elevation.
void RenderEntityShadow(const Vector3& pos, double size, const SurfaceMap2D<double>& heightMap);

} // namespace GameRenderer
