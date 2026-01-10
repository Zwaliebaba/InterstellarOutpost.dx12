#include "pch.h"
#include "debug_render.h"
#include "random_number.h"
#include "particle_system.h"
#include "app.h"
#include "smokemarker.h"

SmokeMarker::SmokeMarker()
  : Building() { m_type = TypeSmokeMarker; }

bool SmokeMarker::Advance()
{
  for (int i = 0; i < 3; ++i)
  {
    Vector3 vel = g_upVector;
    vel.x += sfrand(0.4f);
    vel.z += sfrand(0.4f);
    vel.SetLength(50.0f);

    float size = 200.0f + frand(100.0f);

    g_app->m_particleSystem->CreateParticle(m_pos, vel, Particle::TypeMissileTrail, size);
  }

  return false;
}

void SmokeMarker::RenderAlphas(float _predictionTime)
{
}

bool SmokeMarker::DoesSphereHit(const Vector3& _pos, float _radius) { return false; }

bool SmokeMarker::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool SmokeMarker::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, float _rayLen, Vector3* _pos, Vector3* _norm)
{
  if (g_app->m_editing)
    return Building::DoesRayHit(_rayStart, _rayDir, _rayLen, _pos, _norm);
  return false;
}
