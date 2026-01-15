#include "pch.h"
#include "shaman.h"
#include "app.h"
#include "camera.h"
#include "entity.h"
#include "entity_grid.h"
#include "explosion.h"
#include "gametimer.h"
#include "input.h"
#include "location.h"
#include "main.h"
#include "math_utils.h"
#include "obstruction_grid.h"
#include "particle_system.h"
#include "portal.h"
#include "random_number.h"
#include "resource.h"
#include "shape.h"
#include "soundsystem.h"
#include "team.h"
#include "teleport.h"
#include "text_renderer.h"

#define MAX_EGGS 3

Shaman::Shaman()
  : Entity(),
    m_sacraficeDarwinians(false),
    m_sacraficeTimer(0.0),
    m_summonType(TypeInvalid),
    m_mode(ModeNone),
    m_lastModeChange(0.0),
    m_sacrafices(0),
    m_vunerable(true),
    m_paralyzed(false),
    m_renderTeleportTargets(false)
{
  m_type = TypeShaman;
  m_shape = g_app->m_resource->GetShape("officer.shp");
  m_centrePos = m_shape->CalculateCentre(g_identityMatrix34);
  m_radius = m_shape->CalculateRadius(g_identityMatrix34, m_centrePos);
}

Shaman::~Shaman()
{
  if (m_id.GetTeamId() != 255)
  {
    Team* team = g_app->m_location->m_teams[m_id.GetTeamId()];
    team->UnRegisterSpecial(m_id);
  }
}

void Shaman::Begin()
{
  m_wayPoint = m_pos;

  if (m_id.GetTeamId() != 255)
  {
    Team* team = g_app->m_location->m_teams[m_id.GetTeamId()];
    team->RegisterSpecial(m_id);
  }
}

bool Shaman::Advance(Unit* _unit) { return true; }

bool Shaman::AdvanceToTargetPosition()
{
  if (m_paralyzed)
    return false;
  //
  // Work out where we want to be next

  if ((m_wayPoint - m_pos).Mag() > 5.0)
  {
    double speed = m_stats[StatSpeed];

    double amountToTurn = SERVER_ADVANCE_PERIOD * 6.0;

    Vector3 targetDir = (m_wayPoint - m_pos).Normalise();
    Vector3 actualDir = m_front * (1.0 - amountToTurn) + targetDir * amountToTurn;
    actualDir.Normalise();

    Vector3 oldPos = m_pos;
    Vector3 newPos = m_pos + actualDir * speed * SERVER_ADVANCE_PERIOD;

    //
    // Slow us down if we're going up hill
    // Speed up if going down hill

    double currentHeight = g_app->m_location->m_landscape.m_heightMap->GetValue(oldPos.x, oldPos.z);
    double nextHeight = g_app->m_location->m_landscape.m_heightMap->GetValue(newPos.x, newPos.z);
    double factor = 1.0 - (currentHeight - nextHeight) / -3.0;
    if (factor < 0.1)
      factor = 0.1;
    if (factor > 2.0)
      factor = 2.0;
    newPos = m_pos + actualDir * speed * factor * SERVER_ADVANCE_PERIOD;
    newPos = PushFromObstructions(newPos);

    m_pos = newPos;
    m_vel = (m_pos - oldPos) / SERVER_ADVANCE_PERIOD;
    m_front = (newPos - oldPos).Normalise();

    if (m_onGround && !m_dead)
      m_pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(m_pos.x, m_pos.z);
  }
  else
  {
    m_vel.Zero();
    return true;
  }

  if (m_teleportId != -1)
  {
    int teleportId = EnterTeleports(m_teleportId);
    if (teleportId != -1)
    {
      auto teleport = static_cast<Teleport*>(g_app->m_location->GetBuilding(teleportId));
      Vector3 exitPos, exitFront;
      bool exitFound = teleport->GetExit(exitPos, exitFront);
      if (exitFound)
        m_wayPoint = exitPos + exitFront * 30.0;
      m_teleportId = -1;
    }
  }

  return false;
}

bool Shaman::ChangeHealth(int _amount, int _damageType)
{
  if (m_vunerable)
  {
    bool dead = m_dead;

    if (!m_dead)
    {
      if (_amount < 0)
        g_app->m_soundSystem->TriggerEntityEvent(this, "LoseHealth");

      if (m_stats[StatHealth] + _amount <= 0)
      {
        m_stats[StatHealth] = 100;
        m_dead = true;
        g_app->m_soundSystem->TriggerEntityEvent(this, "Die");
      }
      else if (m_stats[StatHealth] + _amount > 255)
        m_stats[StatHealth] = 255;
      else
        m_stats[StatHealth] += _amount;
    }

    if (!dead && m_dead)
    {
      Matrix34 transform(m_front, g_upVector, m_pos);
      g_explosionManager.AddExplosion(m_shape, transform);
    }
  }
  else
    return false;

  return true;
}

void Shaman::Render(double predictionTime)
{
  if (m_dead)
    return;

  Vector3 predictedPos = m_pos + m_vel * predictionTime;

  Vector3 entityUp = g_upVector;
  Vector3 entityRight(m_front ^ entityUp);
  Vector3 entityFront = entityUp ^ entityRight;

  Matrix34 mat(entityFront, entityUp, predictedPos);

  if (m_renderDamaged)
  {
    double timeIndex = g_gameTime + m_id.GetUniqueId() * 10;
    double thefrand = frand();
    if (thefrand > 0.7)
      mat.f *= (1.0 - sin(timeIndex) * 0.5);
    else if (thefrand > 0.4)
      mat.u *= (1.0 - sin(timeIndex) * 0.2);
    else
      mat.r *= (1.0 - sin(timeIndex) * 0.5);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
  }

  m_shape->Render(predictionTime, mat);

  if (m_sacraficeDarwinians)
  {
    int numSteps = 30;
    double angle = 0.0;
    double size = 50.0;

    RGBAColour colour(128, 128, 128, 255);
    if (m_id.GetTeamId() != 255)
      colour = g_app->m_location->m_teams[m_id.GetTeamId()]->m_colour;

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= numSteps; ++i)
    {
      double alpha = (abs(sin(angle * 2 + g_gameTime)) * 0.65);
      glColor4ub(colour.r, colour.g, colour.b, alpha * 255);

      double xDiff = size * sin(angle);
      double zDiff = size * cos(angle);
      Vector3 pos = predictedPos + Vector3(xDiff, 5, zDiff);
      pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(pos.x, pos.z);
      if (pos.y < 2)
        pos.y = 2;
      glVertex3dv(pos.GetData());
      pos.y += 5;
      glVertex3dv(pos.GetData());
      angle += 2.0 * M_PI / static_cast<double>(numSteps);
    }
    glEnd();

    glEnable(GL_CULL_FACE);
  }

  if (m_mode > ModeNone)
  {
    char msg[256];
    switch (m_mode)
    {
    case ModeCreateDarwinians:
      sprintf(msg, "Creating Darwinians");
      break;
    case ModeFollow:
      sprintf(msg, "Darwinians Following");
      break;
    }

    Vector3 pos = m_pos;
    pos.y += 50.0;
    g_gameFont.DrawText3DCentre(pos, 10.0, msg);
  }

  if (m_renderTeleportTargets)
    RenderTeleportTargets();
}

void Shaman::RenderTeleportTargets()
{
  LList<ShamanTargetArea>* targetAreas = GetTargetArea();
  RGBAColour* colour = &g_app->m_location->GetMyTeam()->m_colour;

  if (targetAreas->Size() == 1 && targetAreas->GetPointer(0)->m_pos == g_zeroVector && targetAreas->GetPointer(0)->m_radius > 9999)
  {
    delete targetAreas;
    return;
  }

  for (int i = 0; i < targetAreas->Size(); ++i)
  {
    ShamanTargetArea* tta = targetAreas->GetPointer(i);

    double angle = g_gameTime * 3.0;
    Vector3 dif(tta->m_radius * sin(angle), 0.0, tta->m_radius * cos(angle));

    Vector3 pos = tta->m_pos + dif;
    pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(pos.x, pos.z) + 5.0;
    g_app->m_particleSystem->CreateParticle(pos, g_zeroVector, Particle::TypeMuzzleFlash, 60.0);

    pos = tta->m_pos - dif;
    pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(pos.x, pos.z) + 5.0;
    g_app->m_particleSystem->CreateParticle(pos, g_zeroVector, Particle::TypeMuzzleFlash, 60.0);
  }

  delete targetAreas;
}

void Shaman::SetWaypoint(const Vector3& _wayPoint)
{
  if (m_paralyzed)
    return;

  m_wayPoint = _wayPoint;

  LList<int>* nearbyBuildings = g_app->m_location->m_obstructionGrid->GetBuildings(_wayPoint.x, _wayPoint.z);
  for (int i = 0; i < nearbyBuildings->Size(); ++i)
  {
    int buildingId = nearbyBuildings->GetData(i);
    Building* building = g_app->m_location->GetBuilding(buildingId);
    if (building->m_type == Building::TypeRadarDish || building->m_type == Building::TypeBridge)
    {
      double distance = (building->m_pos - _wayPoint).Mag();
      auto teleport = static_cast<Teleport*>(building);
      if (distance < 5.0 && teleport->Connected())
      {
        Vector3 entrancePos, entranceFront;
        teleport->GetEntrance(entrancePos, entranceFront);
        m_wayPoint = entrancePos;
        m_teleportId = teleport->m_id.GetUniqueId();
        break;
      }
    }
  }
}

void Shaman::SummonEntity(int _type)
{
  if (m_summonType != TypeInvalid)
    return;
  //if( m_eggQueue.Size() + m_eggs.Size() < MAX_EGGS )	m_eggQueue.PutData( _type );
  //m_eggQueue.PutData( _type );
  BeginSummoning(_type);
}

void Shaman::BeginSummoning(int _type) {}

int Shaman::GetNearestPortal()
{
  double minDistance = 9999.9;
  int id = -1;
  for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
  {
    if (g_app->m_location->m_buildings.ValidIndex(i))
    {
      auto b = static_cast<Portal*>(g_app->m_location->m_buildings[i]);
      if (b && b->m_type == Building::TypePortal)
      {
        //                if( b->m_masterPortal ) continue;

        double distance = (m_pos - b->m_pos).Mag();
        if (distance < minDistance)
        {
          id = i;
          minDistance = distance;
        }
      }
    }
  }

  return id;
}

void Shaman::Paralyze()
{
  m_paralyzed = true;
  m_wayPoint = m_pos;
  m_sacraficeDarwinians = false;
  m_vunerable = false;
  m_stats[StatHealth] = EntityBlueprint::GetStat(TypeShaman, StatHealth);
  m_dead = false;
  m_vel.Zero();
}

int Shaman::GetSummonType() { return m_summonType; }

bool Shaman::IsSacraficing() { return m_sacraficeDarwinians; }

bool Shaman::IsSelectable() { return true; }

bool Shaman::CallingDarwinians() { return (IsSacraficing() || m_mode == ModeFollow); }

void Shaman::ChangeMode(Vector3 _mousePos)
{
  if ((m_pos - _mousePos).Mag() > 20.0)
    m_renderTeleportTargets = true;
  else
  {
    m_renderTeleportTargets = false;
    double timeNow = GetNetworkTime();
    if (timeNow > m_lastModeChange + 0.3)
    {
      m_lastModeChange = timeNow;
      m_mode++;
      if (m_mode >= NumModes)
        m_mode = 0;
    }
  }
}

void Shaman::TeleportToPos(Vector3 _mousePos)
{
  double timeNow = GetNetworkTime();
  if (timeNow > m_lastModeChange + 0.3)
  {
    m_lastModeChange = timeNow;

    if (ValidTeleportPos(_mousePos))
    {
      g_app->m_location->m_entityGrid->RemoveObject(m_id, m_pos.x, m_pos.z, m_radius);
      m_pos = _mousePos;
      m_wayPoint = m_pos;
      m_vel.Zero();
      g_app->m_location->m_entityGrid->AddObject(m_id, m_pos.x, m_pos.z, m_radius);
    }
  }
}

bool Shaman::ValidTeleportPos(Vector3 _pos)
{
  LList<ShamanTargetArea>* targetAreas = GetTargetArea();
  bool success = false;

  for (int i = 0; i < targetAreas->Size(); ++i)
  {
    ShamanTargetArea* targetArea = targetAreas->GetPointer(i);
    if ((_pos - targetArea->m_pos).Mag() <= targetArea->m_radius)
    {
      success = true;
      break;
    }
  }

  delete targetAreas;
  return success;
}

LList<ShamanTargetArea>* Shaman::GetTargetArea()
{
  auto list = new LList<ShamanTargetArea>();

  for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
  {
    if (g_app->m_location->m_buildings.ValidIndex(i))
    {
      Building* b = g_app->m_location->m_buildings[i];
      if (b->m_type == Building::TypePortal && b->m_id.GetTeamId() == m_id.GetTeamId())
      {
        ShamanTargetArea sha;
        sha.m_pos = b->m_pos;
        sha.m_radius = static_cast<Portal*>(b)->m_portalRadius;
        list->PutData(sha);
      }
    }
  }

  return list;
}

bool Shaman::CanSummonEntity(int _entityType)
{
  /*switch( _entityType )
  {
        case Entity::TypeCentipede:         return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 1 );
    case Entity::TypeArmyAnt:           return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 2 );
    case Entity::TypeSoulDestroyer:     return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 5 );
    case Entity::TypeSpider:            return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 1 );
    case Entity::TypeSporeGenerator:    return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 2 );
    case Entity::TypeTripod:            return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 3 );
    case Entity::TypeVirii:             return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 0 );
    case Entity::TypeDarwinian:         return (Portal::s_numPortals[m_id.GetTeamId()+1] >= 2 );

    default: return false;
  }*/
  return false;
}

bool Shaman::CanCapturePortal()
{
  auto p = static_cast<Portal*>(g_app->m_location->GetBuilding(GetNearestPortal()));
  if (p)
  {
    if (p->m_id.GetTeamId() == m_id.GetTeamId())
      return false;
    if ((m_pos - p->m_pos).Mag() > 300.0)
      return false;

    int numFound;
    g_app->m_location->m_entityGrid->GetEnemies(s_neighbours, p->m_pos.x, p->m_pos.z, 300.0, &numFound, m_id.GetTeamId());

    bool monstersFound = false;
    for (int i = 0; i < numFound; ++i)
    {
      Entity* e = g_app->m_location->GetEntity(s_neighbours[i]);
      if (e)
      {
        if (IsSummonable(e->m_type))
        {
          monstersFound = true;
          break;
        }
      }
    }

    if (!monstersFound)
      return true;
  }

  return false;
}

bool Shaman::IsSummonable(int _entityType)
{
  switch (_entityType)
  {
  case TypeCentipede:
  //case Entity::TypeArmyAnt:
  case TypeSoulDestroyer:
  case TypeSpider:
  case TypeSporeGenerator:
  //case Entity::TypeTripod:
  case TypeVirii:
    //case Entity::TypeDarwinian:
    return true;

  default:
    return false;
  }
}
