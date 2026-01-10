#include "pch.h"
#include "math_utils.h"
#include "resource.h"
#include "text_stream_readers.h"
#include "text_file_writer.h"
#include "app.h"
#include "location.h"
#include "main.h"
#include "entity_grid.h"
#include "soundsystem.h"
#include "jumppad.h"
#include "darwinian.h"

#define JUMPPAD_RADIUS 40.0

JumpPad::JumpPad()
  : Building(),
    m_force(0.0),
    m_angle(0.0),
    m_launchTimer(0.0) { m_type = TypeJumpPad; }

void JumpPad::Initialise(Building* _template)
{
  Building::Initialise(_template);
  auto pad = static_cast<JumpPad*>(_template);
  m_force = pad->m_force;
  m_angle = pad->m_angle;
}

bool JumpPad::Advance()
{
  m_launchTimer -= SERVER_ADVANCE_PERIOD;
  if (m_launchTimer <= 0.0)
  {
    m_launchTimer = 0.3;

    Vector3 targetDirection = m_front;
    Vector3 rotation = m_front ^ g_upVector;
    targetDirection.RotateAround(rotation * m_angle);
    targetDirection.SetLength(m_force);

    int numFound;
    g_app->m_location->m_entityGrid->GetNeighbours(s_neighbours, m_pos.x, m_pos.z, JUMPPAD_RADIUS, &numFound);

    for (int i = 0; i < numFound; ++i)
    {
      auto d = static_cast<Darwinian*>(g_app->m_location->GetEntitySafe(s_neighbours[i], Entity::TypeDarwinian));
      if (d && d->m_onGround && d->m_state != Darwinian::StateInsideArmour)
      {
        d->m_vel = targetDirection;
        d->m_onGround = false;
        d->m_state = Darwinian::StateIdle;
        d->m_wayPoint = g_zeroVector;
        d->m_ordersSet = false;
      }
    }
  }
  return false;
}

void JumpPad::Render(double _predictionTime) {}

void JumpPad::RenderAlphas(double _predictionTime)
{
  float c = 255.0f * (0.3f + abs(sinf(g_gameTime) * 0.7f));
  RGBAColour col(c, c, c, 1.0f);

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  double stepSize = 5.0;

  Vector3 from = m_pos - m_front * JUMPPAD_RADIUS;
  Vector3 to = m_pos + m_front * JUMPPAD_RADIUS;

  Vector3 rightAngle = (from - to) ^ g_upVector;
  rightAngle.y = 0;
  rightAngle.SetLength(20.0);

  double distance = (from - to).Mag();
  int numSteps = 2; 

  double timeNow = GetHighResTime() * 5.0;

  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("icons\\thickarrow.bmp"));
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glShadeModel(GL_SMOOTH);
  glBegin(GL_QUAD_STRIP);

  double halfNumSteps = numSteps / 2.0;

  for (int i = 0; i <= numSteps; ++i)
  {
    double alpha = i / halfNumSteps;
    if (i >= halfNumSteps)
      alpha = 1.0 - (i - halfNumSteps) / halfNumSteps;

    alpha *= 255;

    col.a = alpha;
    glColor4ubv(col.GetData());

    Vector3 thisPos = from + (to - from) * (i / static_cast<double>(numSteps));

    Vector3 right = thisPos + rightAngle;
    Vector3 left = thisPos - rightAngle;

    right.y = g_app->m_location->m_landscape.m_heightMap->GetValue(right.x, right.z);
    left.y = g_app->m_location->m_landscape.m_heightMap->GetValue(left.x, left.z);

    glTexCoord2f(0, i - timeNow / 4);
    glVertex3dv((left).GetData());
    glTexCoord2f(1, i - timeNow / 4);
    glVertex3dv((right).GetData());
  }

  glEnd();
  glShadeModel(GL_FLAT);
  glDisable(GL_TEXTURE_2D);

  glLineWidth(1.0);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glEnable(GL_DEPTH_TEST);
}

void JumpPad::Write(TextWriter* _out)
{
  Building::Write(_out);
  _out->printf("%-2.2f ", m_force);
  _out->printf("%-2.2f ", m_angle);
}

void JumpPad::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);
  m_force = atof(_in->GetNextToken());
  m_angle = atof(_in->GetNextToken());
}

bool JumpPad::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool JumpPad::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool JumpPad::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* norm)
{
  return RaySphereIntersection(_rayStart, _rayDir, m_pos, m_radius, _rayLen);
}
