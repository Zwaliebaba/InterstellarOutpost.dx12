#include "pch.h"
#include "resource.h"
#include "shape.h"
#include "text_stream_readers.h"
#include "math_utils.h"
#include "preferences.h"
#include "random_number.h"
#include "researchitem.h"
#include "explosion.h"
#include "main.h"
#include "app.h"
#include "global_world.h"
#include "camera.h"
#include "location.h"
#include "taskmanager_interface.h"
#include "soundsystem.h"

ResearchItem::ResearchItem()
  : Building(),
    m_reprogrammed(100.0),
    m_end1(nullptr),
    m_end2(nullptr),
    m_researchType(-1),
    m_level(1),
    m_percentResearchedSmooth(0.0f)
{
  m_type = TypeResearchItem;
  m_researchType = GlobalResearch::TypeEngineer;

  Building::SetShape(g_app->m_resource->GetShape("researchitem.shp"));

  m_front.RotateAroundY(frand(2.0 * M_PI));

  constexpr char end1Name[] = "MarkerGrab1";
  m_end1 = m_shape->m_rootFragment->LookupMarker(end1Name);
  ASSERT_TEXT(m_end1, "ResearchItem: Can't get Marker(%s) from shape(%s), probably a corrupted file\n", end1Name, m_shape->m_name);

  constexpr char end2Name[] = "MarkerGrab2";
  m_end2 = m_shape->m_rootFragment->LookupMarker(end2Name);
  ASSERT_TEXT(m_end2, "ResearchItem: Can't get Marker(%s) from shape(%s), probably a corrupted file\n", end2Name, m_shape->m_name);
}

void ResearchItem::Initialise(Building* _template)
{
  Building::Initialise(_template);

  m_researchType = static_cast<ResearchItem*>(_template)->m_researchType;
  m_level = static_cast<ResearchItem*>(_template)->m_level;
}

void ResearchItem::SetDetail(int _detail)
{
  m_pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(m_pos.x, m_pos.z);
  m_pos.y += 20.0;

  Matrix34 mat(m_front, m_up, m_pos);
  m_centrePos = m_shape->CalculateCentre(mat);
  m_radius = m_shape->CalculateRadius(mat, m_centrePos);
}

bool ResearchItem::Advance()
{
  if (m_vel.Mag() > 1.0)
  {
    m_pos += m_vel * SERVER_ADVANCE_PERIOD;
    m_pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(m_pos.x, m_pos.z);
    m_vel *= (1.0 - SERVER_ADVANCE_PERIOD * 0.5);

    Matrix34 mat(m_front, g_upVector, m_pos);
    m_centrePos = m_shape->CalculateCentre(mat);
  }
  else
    m_vel.Zero();

  if (m_researchType > -1 && g_app->m_globalWorld->m_research->HasResearch(m_researchType) && g_app->m_globalWorld->m_research->
    CurrentLevel(m_researchType) >= m_level)
    return true;

  if (m_reprogrammed <= 0.0)
  {
    Matrix34 mat(m_front, m_up, m_pos);
    g_explosionManager.AddExplosion(m_shape, mat, 1.0);

    int existingLevel = g_app->m_globalWorld->m_research->CurrentLevel(m_researchType);

    g_app->m_globalWorld->m_research->AddResearch(m_researchType);
    g_app->m_globalWorld->m_research->m_researchLevel[m_researchType] = m_level;

    g_app->m_soundSystem->TriggerBuildingEvent(this, "AquireResearch");

    if (existingLevel == 0)
      g_app->m_taskManagerInterface->SetCurrentMessage(TaskManagerInterface::MessageResearch, m_researchType, 4.0);
    else
      g_app->m_taskManagerInterface->SetCurrentMessage(TaskManagerInterface::MessageResearchUpgrade, m_researchType, 4.0);

    return true;
  }

  return false;
}

bool ResearchItem::NeedsReprogram() { return (m_reprogrammed > 0.0); }

double ResearchItem::GetReprogrammed() { return m_reprogrammed; }

bool ResearchItem::Reprogram()
{
  m_reprogrammed -= SERVER_ADVANCE_PERIOD * 3.0;

  return (m_reprogrammed <= 0.0);
}

void ResearchItem::GetEndPositions(Vector3& _end1, Vector3& _end2)
{
  Matrix34 mat(m_front, m_up, m_pos);

  _end1 = m_end1->GetWorldMatrix(mat).pos;
  _end2 = m_end2->GetWorldMatrix(mat).pos;
}

void ResearchItem::Render(double _predictionTime)
{
  if (m_reprogrammed <= 0.0)
    return;

  Vector3 rotateAround = g_upVector;
  rotateAround.RotateAroundX(g_gameTime * 1.0);
  rotateAround.RotateAroundZ(g_gameTime * 0.7);
  rotateAround.Normalise();

  m_front.RotateAround(rotateAround * g_advanceTime);
  m_up.RotateAround(rotateAround * g_advanceTime);

  Vector3 predictedPos = m_pos + m_vel * _predictionTime;
  Matrix34 mat(m_front, m_up, predictedPos);

  m_shape->Render(0.0, mat);
}

void ResearchItem::RenderAlphas(double _predictionTime)
{
  Building::RenderAlphas(_predictionTime);

  Vector3 camUp = g_app->m_camera->GetUp();
  Vector3 camRight = g_app->m_camera->GetRight();

  glDepthMask(false);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\cloudyglow.bmp"));

  double timeIndex = g_gameTime + m_id.GetUniqueId() * 10.0;

  int buildingDetail = g_prefsManager->GetInt("RenderBuildingDetail", 1);
  int maxBlobs = 20;
  if (buildingDetail == 2)
    maxBlobs = 10;
  if (buildingDetail == 3)
    maxBlobs = 0;

  double alpha = 1.0;

  for (int i = 0; i < maxBlobs; ++i)
  {
    Vector3 pos = m_centrePos;
    pos.x += sin(timeIndex + i) * i * 0.3;
    pos.y += cos(timeIndex + i) * sin(i * 10) * 5;
    pos.z += cos(timeIndex + i) * i * 0.3;

    double size = 5.0 + sin(timeIndex + i * 10) * 7.0;
    size = max(size, 2.0);

    //glColor4f( 0.6, 0.2, 0.1, alpha);
    glColor4f(0.1, 0.2, 0.8, alpha);

    glBegin(GL_QUADS);
    glTexCoord2i(0, 0);
    glVertex3dv((pos - camRight * size + camUp * size).GetData());
    glTexCoord2i(1, 0);
    glVertex3dv((pos + camRight * size + camUp * size).GetData());
    glTexCoord2i(1, 1);
    glVertex3dv((pos + camRight * size - camUp * size).GetData());
    glTexCoord2i(0, 1);
    glVertex3dv((pos - camRight * size - camUp * size).GetData());
    glEnd();
  }

  //
  // Starbursts

  alpha = 1.0 - m_reprogrammed / 100.0;
  alpha *= 0.3;

  glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\starburst.bmp"));

  if (alpha > 0.0)
  {
    int numStars = 10;
    if (buildingDetail == 2)
      numStars = 5;
    if (buildingDetail == 3)
      numStars = 2;

    for (int i = 0; i < numStars; ++i)
    {
      Vector3 pos = m_centrePos;
      pos.x += sin(timeIndex + i) * i * 0.3;
      pos.y += (cos(timeIndex + i) * cos(i * 10) * 2);
      pos.z += cos(timeIndex + i) * i * 0.3;

      double size = i * 10 * alpha;
      if (i > numStars - 2)
        size = i * 20 * alpha;

      //glColor4f( 1.0, 0.4, 0.2, alpha );
      glColor4f(0.1, 0.2, 0.8, alpha);

      glBegin(GL_QUADS);
      glTexCoord2i(0, 0);
      glVertex3dv((pos - camRight * size + camUp * size).GetData());
      glTexCoord2i(1, 0);
      glVertex3dv((pos + camRight * size + camUp * size).GetData());
      glTexCoord2i(1, 1);
      glVertex3dv((pos + camRight * size - camUp * size).GetData());
      glTexCoord2i(0, 1);
      glVertex3dv((pos - camRight * size - camUp * size).GetData());
      glEnd();
    }
  }

  //
  // Draw control line to heaven

  alpha = 1.0 - m_reprogrammed / 100.0;
  alpha *= (0.5 + abs(cos(g_gameTime)) * 0.5);

  if (alpha > 0.0)
  {
    glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\laser.bmp"));
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    double w = 10.0 * alpha;

    glBegin(GL_QUADS);
    glColor4f(0.1, 0.2, 0.8, alpha);
    glTexCoord2i(0, 0);
    glVertex3dv((m_pos + Vector3(0, -50, 0) - camRight * w).GetData());
    glTexCoord2i(0, 1);
    glVertex3dv((m_pos + Vector3(0, -50, 0) + camRight * w).GetData());

    glColor4f(0.1, 0.2, 0.8, 0.0);
    glTexCoord2i(1, 1);
    glVertex3dv((m_pos + Vector3(0, 1000, 0) + camRight * w).GetData());
    glTexCoord2i(1, 0);
    glVertex3dv((m_pos + Vector3(0, 1000, 0) - camRight * w).GetData());
    glEnd();

    w *= 0.3;

    glBegin(GL_QUADS);
    glColor4f(0.1, 0.2, 0.8, alpha);
    glTexCoord2i(0, 0);
    glVertex3dv((m_pos + Vector3(0, -50, 0) - camRight * w).GetData());
    glTexCoord2i(0, 1);
    glVertex3dv((m_pos + Vector3(0, -50, 0) + camRight * w).GetData());

    glColor4f(0.1, 0.2, 0.8, 0.0);
    glTexCoord2i(1, 1);
    glVertex3dv((m_pos + Vector3(0, 1000, 0) + camRight * w).GetData());
    glTexCoord2i(1, 0);
    glVertex3dv((m_pos + Vector3(0, 1000, 0) - camRight * w).GetData());
    glEnd();
  }

  glShadeModel(GL_FLAT);
  glDisable(GL_TEXTURE_2D);
  glDepthMask(true);

}

void ResearchItem::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);

  m_researchType = GlobalResearch::GetType(_in->GetNextToken());

  if (_in->TokenAvailable())
    m_level = atoi(_in->GetNextToken());
}

bool ResearchItem::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool ResearchItem::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool ResearchItem::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* norm)
{
  return RaySphereIntersection(_rayStart, _rayDir, m_pos, m_radius, _rayLen);
}

bool ResearchItem::IsInView()
{
  if (Building::IsInView())
    return true;

  if (g_app->m_camera->PosInViewFrustum(m_pos + Vector3(0, g_app->m_camera->GetPos().y, 0)))
    return true;

  return false;
}
