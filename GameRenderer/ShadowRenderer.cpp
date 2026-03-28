#include "pch.h"

#include "ShadowRenderer.h"

#include "2d_surface_map.h"
#include "camera.h"
#include "opengl_directx.h"
#include "renderer.h"
#include "resource.h"

namespace GameRenderer {

double BeginShadowPass(Resource& resource, Renderer& renderer, Camera& camera) {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, resource.GetTexture("textures\\glow.bmp"));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glDisable(GL_CULL_FACE);
  glDepthMask(false);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
  glColor4f(0.6, 0.6, 0.6, 0.0);

  double nearPlane = renderer.GetNearPlane();
  camera.SetupProjectionMatrix(static_cast<float>(nearPlane * 1.05), renderer.GetFarPlane());
  return nearPlane;
}

void EndShadowPass(double savedNearPlane, Renderer& renderer, Camera& camera) {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);

  glDepthMask(true);
  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);

  camera.SetupProjectionMatrix(static_cast<float>(savedNearPlane), renderer.GetFarPlane());
}

void RenderEntityShadow(const Vector3& pos, double size, const SurfaceMap2D<double>& heightMap) {
  auto shadowR = Vector3(size, 0, 0);
  auto shadowU = Vector3(0, 0, size);

  Vector3 posA = pos - shadowR - shadowU;
  Vector3 posB = pos + shadowR - shadowU;
  Vector3 posC = pos + shadowR + shadowU;
  Vector3 posD = pos - shadowR + shadowU;

  posA.y = heightMap.GetValue(posA.x, posA.z) + 0.9;
  posB.y = heightMap.GetValue(posB.x, posB.z) + 0.9;
  posC.y = heightMap.GetValue(posC.x, posC.z) + 0.9;
  posD.y = heightMap.GetValue(posD.x, posD.z) + 0.9;

  posA.y = max(posA.y, 1.0);
  posB.y = max(posB.y, 1.0);
  posC.y = max(posC.y, 1.0);
  posD.y = max(posD.y, 1.0);

  if (posA.y > pos.y && posB.y > pos.y && posC.y > pos.y && posD.y > pos.y) {
    // The object casting the shadow is beneath the ground
    return;
  }

  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3dv(posA.GetData());
  glTexCoord2f(1.0, 0.0);
  glVertex3dv(posB.GetData());
  glTexCoord2f(1.0, 1.0);
  glVertex3dv(posC.GetData());
  glTexCoord2f(0.0, 1.0);
  glVertex3dv(posD.GetData());
  glEnd();
}

} // namespace GameRenderer
