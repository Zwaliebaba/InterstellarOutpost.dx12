#include "pch.h"

#include <math.h>

#include "debug_render.h"

#include "text_file_writer.h"
#include "math_utils.h"
#include "matrix34.h"
#include "resource.h"
#include "shape.h"
#include "text_stream_readers.h"
#include "preferences.h"
#include "profiler.h"
#include "text_renderer.h"
#include "language_table.h"
#include "random_number.h"

#include "app.h"
#include "camera.h"
#include "globals.h"
#include "location.h"
#include "renderer.h"
#include "team.h"
#include "unit.h"
#include "entity_grid.h"
#include "obstruction_grid.h"
#include "particle_system.h"
#include "explosion.h"

#include "soundsystem.h"

#include "clienttoserver.h"

#include "building.h"
#include "cave.h"
#include "factory.h"
#include "radardish.h"
#include "laserfence.h"
#include "controltower.h"
#include "gunturret.h"
#include "bridge.h"
#include "powerstation.h"
#include "tree.h"
#include "wall.h"
#include "trunkport.h"
#include "library.h"
#include "generator.h"
#include "mine.h"
#include "constructionyard.h"
#include "upgrade_port.h"
#include "incubator.h"
#include "darwinian.h"
#include "anthill.h"
#include "safearea.h"
#include "triffid.h"
#include "spiritreceiver.h"
#include "spawnpoint.h"
#include "blueprintstore.h"
#include "ai.h"
#include "researchitem.h"
#include "scripttrigger.h"
#include "spam.h"
#include "goddish.h"
#include "staticshape.h"
#include "rocket.h"
#include "switch.h"
#include "generichub.h"
#include "feedingtube.h"
#include "multiwiniazone.h"
#include "carryablebuilding.h"
#include "chess.h"
#include "clonelab.h"
#include "controlstation.h"
#include "portal.h"
#include "crate.h"
#include "statue.h"
#include "wallbuster.h"
#include "pulsebomb.h"
#include "restrictionzone.h"
#include "jumppad.h"
#include "aiobjective.h"
#include "eruptionmarker.h"
#include "smokemarker.h"

Shape* Building::s_controlPad = nullptr;
ShapeMarker* Building::s_controlPadStatus = nullptr;

std::vector<WorldObjectId> Building::s_neighbours;

Building::Building()
  : m_front(1, 0, 0),
    m_timeOfDeath(-1.0),
    m_dynamic(false),
    m_isGlobal(false),
    m_radius(13.0),
    m_destroyed(false),
    m_shape(nullptr)
{
  if (!s_controlPad)
  {
    s_controlPad = g_app->m_resource->GetShape("controlpad.shp");
    DEBUG_ASSERT(s_controlPad);

    constexpr char controlPadStatusName[] = "MarkerStatus";
    s_controlPadStatus = s_controlPad->m_rootFragment->LookupMarker(controlPadStatusName);
    ASSERT_TEXT(s_controlPadStatus, "Building: Can't get Marker(%s) from shape(%s), probably a corrupted file\n", controlPadStatusName,
                s_controlPad->m_name);
  }

  if (g_app->IsSinglePlayer())
    m_id.SetTeamId(1);
  else
    m_id.SetTeamId(255);

  m_up = g_upVector;
}

Building::~Building() { m_ports.EmptyAndDelete(); }

void Building::Initialise(Building* _template)
{
  m_id = _template->m_id;
  m_pos = _template->m_pos;
  m_pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(m_pos.x, m_pos.z);
  m_front = _template->m_front;
  m_up = _template->m_up;
  m_dynamic = _template->m_dynamic;
  m_isGlobal = _template->m_isGlobal;
  m_destroyed = _template->m_destroyed;

  if (m_shape)
  {
    Matrix34 mat(m_front, m_up, m_pos);
    m_centrePos = m_shape->CalculateCentre(mat);
    m_radius = m_shape->CalculateRadius(mat, m_centrePos);

    SetShapeLights(m_shape->m_rootFragment);
    SetShapePorts(m_shape->m_rootFragment);
  }
  else
  {
    m_centrePos = m_pos;
    m_radius = 13.0;
  }

  GlobalBuilding* gb = g_app->m_globalWorld->GetBuilding(m_id.GetUniqueId(), g_app->m_requestedLocationId);
  if (gb)
    m_id.SetTeamId(gb->m_teamId);

  g_app->m_soundSystem->TriggerBuildingEvent(this, "Create");
}

void Building::SetDetail(int _detail)
{
  m_pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(m_pos.x, m_pos.z);

  if (m_shape)
  {
    Matrix34 mat(m_front, m_up, m_pos);
    m_centrePos = m_shape->CalculateCentre(mat);
    m_radius = m_shape->CalculateRadius(mat, m_centrePos);

    m_ports.EmptyAndDelete();
    SetShapePorts(m_shape->m_rootFragment);
  }
  else
  {
    m_centrePos = m_pos;
    m_radius = 13.0;
  }
}

bool Building::Advance()
{
  if (m_destroyed)
    return true;

  EvaluatePorts();
  return false;
}

void Building::SetShape(Shape* _shape) { m_shape = _shape; }

void Building::SetShapeLights(ShapeFragment* _fragment)
{
  //
  // Enter all lights from this fragment

  int i;

  for (i = 0; i < _fragment->m_childMarkers.Size(); ++i)
  {
    ShapeMarker* marker = _fragment->m_childMarkers[i];
    if (strstr(marker->m_name, "MarkerLight"))
      m_lights.PutData(marker);
  }

  //
  // Recurse to all child fragments

  for (i = 0; i < _fragment->m_childFragments.Size(); ++i)
  {
    ShapeFragment* fragment = _fragment->m_childFragments[i];
    SetShapeLights(fragment);
  }
}

void Building::SetShapePorts(ShapeFragment* _fragment)
{
  //
  // Enter all ports from this fragment

  int i;

  Matrix34 buildingMat(m_front, m_up, m_pos);

  for (i = 0; i < _fragment->m_childMarkers.Size(); ++i)
  {
    ShapeMarker* marker = _fragment->m_childMarkers[i];
    if (strstr(marker->m_name, "MarkerPort"))
    {
      auto port = new BuildingPort();
      port->m_marker = marker;
      port->m_mat = marker->GetWorldMatrix(buildingMat);
      port->m_mat.pos = PushFromBuilding(port->m_mat.pos, 5.0);
      port->m_mat.pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(port->m_mat.pos.x, port->m_mat.pos.z);

      for (int t = 0; t < NUM_TEAMS; ++t)
        port->m_counter[t] = 0;

      m_ports.PutData(port);
    }
  }

  //
  // Recurse to all child fragments

  for (i = 0; i < _fragment->m_childFragments.Size(); ++i)
  {
    ShapeFragment* fragment = _fragment->m_childFragments[i];
    SetShapePorts(fragment);
  }
}

void Building::Reprogram(double _complete) {}

void Building::ReprogramComplete()
{
  g_app->m_soundSystem->TriggerBuildingEvent(this, "ReprogramComplete");

  GlobalBuilding* gb = g_app->m_globalWorld->GetBuilding(m_id.GetUniqueId(), g_app->m_locationId);
  if (gb)
    gb->m_online = !gb->m_online;

  g_app->m_globalWorld->EvaluateEvents();
}

void Building::SetTeamId(int _teamId)
{
  if (m_id.GetTeamId() != _teamId)
  {
    m_id.SetTeamId(_teamId);

    GlobalBuilding* gb = g_app->m_globalWorld->GetBuilding(m_id.GetUniqueId(), g_app->m_locationId);
    if (gb)
      gb->m_teamId = _teamId;

    g_app->m_soundSystem->TriggerBuildingEvent(this, "ChangeTeam");
  }
}

Vector3 Building::PushFromBuilding(const Vector3& pos, double _radius)
{
  START_PROFILE("PushFromBuilding");

  Vector3 result = pos;

  bool hit = false;
  if (DoesSphereHit(result, _radius))
    hit = true;

  if (hit)
  {
    Vector3 pushForce = (m_pos - result).SetLength(2.0);
    while (DoesSphereHit(result, _radius)) { result -= pushForce; }
  }

  END_PROFILE("PushFromBuilding");

  return result;
}

bool Building::IsInView() { return (g_app->m_camera->SphereInViewFrustum(m_centrePos, m_radius)); }

bool Building::PerformDepthSort(Vector3& _centrePos) { return false; }

void Building::Render(double predictionTime)
{
#ifdef DEBUG_RENDER_ENABLED
  if (g_app->m_editing)
  {
    Vector3 pos(m_pos);
    pos.y += 5.0;
    RenderArrow(pos, pos + m_front * 20.0, 4.0);
  }
#endif

  if (m_shape)
  {
    Matrix34 mat(m_front, m_up, m_pos);
    m_shape->Render(predictionTime, mat);

    //m_shape->RenderMarkers(mat);
  }
}

void Building::RenderAlphas(double predictionTime)
{
  RenderLights();
  RenderPorts();

  //RenderHitCheck();
  //RenderSphere(m_pos, 300);		

  //	if (m_shape)
  //	{
  //		Matrix34 mat(m_front, g_upVector, m_pos);
  //		m_shape->RenderMarkers(mat);
  //	}
}

void Building::RenderLights()
{
  if (m_id.GetTeamId() != 255 && m_lights.Size() > 0)
  {
    if ((g_app->m_clientToServer->m_lastValidSequenceIdFromServer % 10) / 2 == m_id.GetTeamId() || g_app->m_editing)
    {
      for (int i = 0; i < m_lights.Size(); ++i)
      {
        ShapeMarker* marker = m_lights[i];
        Matrix34 rootMat(m_front, m_up, m_pos);
        Matrix34 worldMat = marker->GetWorldMatrix(rootMat);
        Vector3 lightPos = worldMat.pos;

        double signalSize = 6.0;
        Vector3 camR = g_app->m_camera->GetRight();
        Vector3 camU = g_app->m_camera->GetUp();

        if (m_id.GetTeamId() == 255)
          glColor3f(0.5, 0.5, 0.5);
        else
          glColor3ubv(g_app->m_location->m_teams[m_id.GetTeamId()]->m_colour.GetData());

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\starburst.bmp"));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glDisable(GL_CULL_FACE);
        glDepthMask(false);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        for (int i = 0; i < 10; ++i)
        {
          double size = signalSize * static_cast<double>(i) / 10.0;
          glBegin(GL_QUADS);
          glTexCoord2f(0.0, 0.0);
          glVertex3dv((lightPos - camR * size - camU * size).GetData());
          glTexCoord2f(1.0, 0.0);
          glVertex3dv((lightPos + camR * size - camU * size).GetData());
          glTexCoord2f(1.0, 1.0);
          glVertex3dv((lightPos + camR * size + camU * size).GetData());
          glTexCoord2f(0.0, 1.0);
          glVertex3dv((lightPos - camR * size + camU * size).GetData());
          glEnd();
        }

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);

        glDepthMask(true);
        glEnable(GL_CULL_FACE);
        glDisable(GL_TEXTURE_2D);
      }
    }
  }
}

void Building::EvaluatePorts()
{
  for (int i = 0; i < GetNumPorts(); ++i)
  {
    BuildingPort* port = m_ports[i];
    //port->m_mat = port->m_marker->GetWorldMatrix(buildingMat);
    port->m_occupant.SetInvalid();

    //
    // Look for a valid Darwinian near the port

    int numFound;
    if (g_app->m_location->m_entityGrid)
    {
      g_app->m_location->m_entityGrid->GetNeighbours(s_neighbours, port->m_mat.pos.x, port->m_mat.pos.z, 5.0, &numFound);
      for (int i = 0; i < numFound; ++i)
      {
        WorldObjectId id = s_neighbours[i];
        Entity* entity = g_app->m_location->GetEntity(id);
        if (entity && entity->m_type == Entity::TypeDarwinian)
        {
          auto darwinian = static_cast<Darwinian*>(entity);
          if (darwinian->m_state == Darwinian::StateOperatingPort)
          {
            port->m_occupant = id;
            break;
          }
        }
      }
    }

    //
    // Update the operation counter

    for (int t = 0; t < NUM_TEAMS; ++t)
    {
      if (port->m_occupant.IsValid())
        port->m_counter[t] = 0;
      else
      {
        port->m_counter[t] -= 4;
        port->m_counter[t] = max(port->m_counter[t], 0);
      }
    }
  }
}

void Building::RenderPorts()
{
  START_PROFILE("RenderPorts");

  int buildingDetail = g_prefsManager->GetInt("RenderBuildingDetail");

  for (int i = 0; i < GetNumPorts(); ++i)
  {
    Vector3 portPos;
    Vector3 portFront;
    GetPortPosition(i, portPos, portFront);

    //
    // Render the port shape

    portPos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(portPos.x, portPos.z) + 0.5;
    Vector3 portUp = g_upVector;
    Matrix34 mat(portFront, portUp, portPos);

    if (buildingDetail < 3)
    {
      g_app->m_renderer->SetObjectLighting();
      s_controlPad->Render(0.0, mat);
      g_app->m_renderer->UnsetObjectLighting();
    }

    //
    // Render the status light

    double size = 6.0;

    Vector3 camR = g_app->m_camera->GetRight() * size;
    Vector3 camU = g_app->m_camera->GetUp() * size;

    Vector3 statusPos = s_controlPadStatus->GetWorldMatrix(mat).pos;

    if (GetPortOccupant(i).IsValid())
      glColor4f(0.3, 1.0, 0.3, 1.0);
    else
      glColor4f(1.0, 0.3, 0.3, 1.0);

    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\starburst.bmp"));
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0);
    glVertex3dv((statusPos - camR - camU).GetData());
    glTexCoord2i(1, 0);
    glVertex3dv((statusPos + camR - camU).GetData());
    glTexCoord2i(1, 1);
    glVertex3dv((statusPos + camR + camU).GetData());
    glTexCoord2i(0, 1);
    glVertex3dv((statusPos - camR + camU).GetData());
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glDepthMask(true);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
  }

  END_PROFILE("RenderPorts");
}

void Building::RenderHitCheck()
{
#ifdef DEBUG_RENDER_ENABLED
  if (m_shape)
  {
    Matrix34 mat(m_front, m_up, m_pos);
    m_shape->RenderHitCheck(mat);
  }
  else
    RenderSphere(m_pos, m_radius);
#endif
}

void Building::RenderLink()
{
#ifdef DEBUG_RENDER_ENABLED
  int buildingId = GetBuildingLink();
  if (buildingId != -1)
  {
    Building* linkBuilding = g_app->m_location->GetBuilding(buildingId);
    if (linkBuilding)
    {
      Vector3 start = m_pos;
      start.y += 10.0;
      Vector3 end = linkBuilding->m_pos;
      end.y += 10.0;
      RenderArrow(start, end, 6.0, RGBAColour(255, 0, 255));
    }
  }
#endif
}

void Building::Damage(double _damage) { g_app->m_soundSystem->TriggerBuildingEvent(this, "Damage"); }

void Building::Destroy(double _intensity)
{
  m_destroyed = true;

  Matrix34 mat(m_front, g_upVector, m_pos);
  for (int i = 0; i < 3; ++i)
    g_explosionManager.AddExplosion(m_shape, mat);

  g_app->m_location->Bang(m_pos, _intensity, _intensity / 4.0);

  g_app->m_soundSystem->TriggerBuildingEvent(this, "Explode");

  for (int i = 0; i < static_cast<int>(_intensity / 4.0); ++i)
  {
    Vector3 vel(0.0, 0.0, 0.0);
    vel.x += syncsfrand(100.0);
    vel.y += syncsfrand(100.0);
    vel.z += syncsfrand(100.0);
    g_app->m_particleSystem->CreateParticle(m_pos, vel, Particle::TypeExplosionCore, 100.0);
  }
}

bool Building::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* norm)
{
  if (m_shape)
  {
    RayPackage ray(_rayStart, _rayDir, _rayLen);
    Matrix34 transform(m_front, m_up, m_pos);
    return m_shape->RayHit(&ray, transform, true);
  }
  return RaySphereIntersection(_rayStart, _rayDir, m_pos, m_radius, _rayLen);
}

bool Building::DoesSphereHit(const Vector3& _pos, double _radius)
{
  if (m_shape)
  {
    SpherePackage sphere(_pos, _radius);
    Matrix34 transform(m_front, m_up, m_pos);
    return m_shape->SphereHit(&sphere, transform);
  }
  double distance = (_pos - m_pos).Mag();
  return (distance <= _radius + m_radius);
}

bool Building::DoesShapeHit(Shape* _shape, Matrix34 _theTransform)
{
  if (m_shape)
  {
    Matrix34 ourTransform(m_front, m_up, m_pos);
    //return m_shape->ShapeHit( _shape, _theTransform, ourTransform, true );
    return _shape->ShapeHit(m_shape, ourTransform, _theTransform, true);
  }
  SpherePackage package(m_pos, m_radius);
  return _shape->SphereHit(&package, _theTransform, true);
}

int Building::GetBuildingLink() { return -1; }

void Building::SetBuildingLink(int _buildingId) {}

int Building::GetNumPorts() { return m_ports.Size(); }

int Building::GetNumPortsOccupied()
{
  int result = 0;
  for (int i = 0; i < GetNumPorts(); ++i)
    if (GetPortOccupant(i).IsValid())
      result++;
  return result;
}

WorldObjectId Building::GetPortOccupant(int _portId)
{
  if (m_ports.ValidIndex(_portId))
  {
    BuildingPort* port = m_ports[_portId];
    return port->m_occupant;
  }

  return WorldObjectId();
}

bool Building::GetPortPosition(int _portId, Vector3& _pos, Vector3& _front)
{
  if (m_ports.ValidIndex(_portId))
  {
    BuildingPort* port = m_ports[_portId];
    _pos = port->m_mat.pos;
    _front = port->m_mat.f;
    return true;
  }

  return false;
}

void Building::OperatePort(int _portId, int _teamId)
{
  if (m_ports.ValidIndex(_portId))
  {
    BuildingPort* port = m_ports[_portId];
    port->m_counter[_teamId]++;
    port->m_counter[_teamId] = min(port->m_counter[_teamId], 50);
  }
}

int Building::GetPortOperatorCount(int _portId, int _teamId)
{
  if (m_ports.ValidIndex(_portId))
  {
    BuildingPort* port = m_ports[_portId];
    return port->m_counter[_teamId];
  }

  return -1;
}

void Building::GetObjectiveCounter(UnicodeString& _dest) { _dest = UnicodeString(); }

void Building::Read(TextReader* _in, bool _dynamic)
{
  char* word;

  int buildingId;
  int teamId;

  word = _in->GetNextToken();
  buildingId = atoi(word);
  word = _in->GetNextToken();
  m_pos.x = atof(word);
  word = _in->GetNextToken();
  m_pos.z = atof(word);
  word = _in->GetNextToken();
  teamId = atoi(word);
  word = _in->GetNextToken();
  m_front.x = atof(word);
  word = _in->GetNextToken();
  m_front.z = atof(word);
  word = _in->GetNextToken();
  m_isGlobal = static_cast<bool>(atoi(word));

  m_front.Normalise();
  m_id.Set(teamId, UNIT_BUILDINGS, -1, buildingId);
  m_dynamic = _dynamic;
}

void Building::Write(TextWriter* _out)
{
  _out->printf("\t%-20s", GetTypeName(m_type));

  _out->printf("%-8d", m_id.GetUniqueId());
  _out->printf("%-8.2f", m_pos.x);
  _out->printf("%-8.2f", m_pos.z);
  _out->printf("%-8d", m_id.GetTeamId());
  _out->printf("%-8.2f", m_front.x);
  _out->printf("%-8.2f", m_front.z);
  _out->printf("%-8d", m_isGlobal);
}

bool Building::BuildingIsBlocked(int _buildingId)
{
#ifdef BLOCK_OLD_DARWINIA_OBJECTS
  return (_buildingId == TypeBlueprintConsole || _buildingId == TypeBlueprintRelay || _buildingId == TypeBlueprintStore || _buildingId ==
    TypeBridge || _buildingId == TypeCave || _buildingId == TypeChessBase || _buildingId == TypeChessPiece || _buildingId == TypeCrate ||
    _buildingId == TypeDisplayScreen || _buildingId == TypeDynamicHub || _buildingId == TypeDynamicNode || _buildingId == TypeFactory ||
    _buildingId == TypeFenceSwitch || _buildingId == TypeGenerator || _buildingId == TypeGodDish || _buildingId == TypeLibrary ||
    _buildingId == TypeMine || _buildingId == TypePortal || _buildingId == TypePowerstation || _buildingId == TypePrimaryUpgradePort ||
    _buildingId == TypeReceiverLink || _buildingId == TypeReceiverSpiritSpawner || _buildingId == TypeRefinery || _buildingId ==
    TypeResearchItem || _buildingId == TypeSafeArea || _buildingId == TypeScriptTrigger || _buildingId == TypeSpam || _buildingId ==
    TypeSpiritProcessor || _buildingId == TypeSpiritReceiver || _buildingId == TypeStatue || _buildingId == TypeTrackEnd || _buildingId ==
    TypeTrackJunction || _buildingId == TypeTrackLink || _buildingId == TypeTrackStart || _buildingId == TypeUpgradePort || _buildingId ==
    TypeYard || _buildingId == TypePylonStart || _buildingId == TypeFeedingTube || _buildingId == TypeAISpawnPoint);
#endif
  return false;
}

Building* Building::CreateBuilding(char* _name)
{
  for (int i = 0; i < NumBuildingTypes; ++i)
  {
    if (BuildingIsBlocked(i))
      continue;
    if (stricmp(_name, GetTypeName(i)) == 0)
      return CreateBuilding(i);
  }

  //DEBUG_ASSERT(false);
  return nullptr;
}

Building* Building::CreateBuilding(int _type)
{
  Building* building = nullptr;

  switch (_type)
  {
  case TypeFactory:
    building = new Factory();
    break;
  case TypeCave:
    building = new Cave();
    break;
  case TypeRadarDish:
    building = new RadarDish();
    break;
  case TypeLaserFence:
    building = new LaserFence();
    break;
  case TypeControlTower:
    building = new ControlTower();
    break;
  case TypeGunTurret:
    building = new GunTurret();
    break;
  case TypeBridge:
    building = new Bridge();
    break;
  case TypePowerstation:
    building = new Powerstation();
    break;
  case TypeTree:
    building = new Tree();
    break;
  case TypeWall:
    building = new Wall();
    break;
  case TypeTrunkPort:
    building = new TrunkPort();
    break;
  case TypeResearchItem:
    building = new ResearchItem();
    break;
  case TypeLibrary:
    building = new Library();
    break;
  case TypeGenerator:
    building = new Generator();
    break;
  case TypePylon:
    building = new Pylon();
    break;
  case TypePylonStart:
    building = new PylonStart();
    break;
  case TypePylonEnd:
    building = new PylonEnd();
    break;
  case TypeSolarPanel:
    building = new SolarPanel();
    break;
  case TypePowerSplitter:
    building = new PowerSplitter();
    break;
  case TypeTrackLink:
    building = new TrackLink();
    break;
  case TypeTrackJunction:
    building = new TrackJunction();
    break;
  case TypeTrackStart:
    building = new TrackStart();
    break;
  case TypeTrackEnd:
    building = new TrackEnd();
    break;
  case TypeRefinery:
    building = new Refinery();
    break;
  case TypeMine:
    building = new Mine();
    break;
  case TypeYard:
    building = new ConstructionYard();
    break;
  case TypeDisplayScreen:
    building = new DisplayScreen();
    break;
  case TypeUpgradePort:
    building = new UpgradePort;
    break;
  case TypePrimaryUpgradePort:
    building = new PrimaryUpgradePort();
    break;
  case TypeIncubator:
    building = new Incubator();
    break;
  case TypeAntHill:
    building = new AntHill();
    break;
  case TypeSafeArea:
    building = new SafeArea();
    break;
  case TypeTriffid:
    building = new Triffid();
    break;
  case TypeSpiritReceiver:
    building = new SpiritReceiver();
    break;
  case TypeReceiverLink:
    building = new ReceiverLink();
    break;
  case TypeReceiverSpiritSpawner:
    building = new ReceiverSpiritSpawner();
    break;
  case TypeSpiritProcessor:
    building = new SpiritProcessor();
    break;
  case TypeSpawnPoint:
    building = new SpawnPoint();
    break;
  case TypeSpawnPopulationLock:
    building = new SpawnPopulationLock();
    break;
  case TypeSpawnPointMaster:
    building = new MasterSpawnPoint();
    break;
  case TypeSpawnLink:
    building = new SpawnLink();
    break;
  case TypeBlueprintStore:
    building = new BlueprintStore();
    break;
  case TypeBlueprintConsole:
    building = new BlueprintConsole();
    break;
  case TypeBlueprintRelay:
    building = new BlueprintRelay();
    break;
  case TypeAITarget:
    building = new AITarget();
    break;
  case TypeAISpawnPoint:
    building = new AISpawnPoint();
    break;
  case TypeScriptTrigger:
    building = new ScriptTrigger();
    break;
  case TypeSpam:
    building = new Spam();
    break;
  case TypeGodDish:
    building = new GodDish();
    break;
  case TypeStaticShape:
    building = new StaticShape();
    break;
  case TypeFuelGenerator:
    building = new FuelGenerator();
    break;
  case TypeFuelPipe:
    building = new FuelPipe();
    break;
  case TypeFuelStation:
    building = new FuelStation();
    break;
  case TypeEscapeRocket:
    building = new EscapeRocket();
    break;
  case TypeFenceSwitch:
    building = new FenceSwitch();
    break;
  case TypeDynamicHub:
    building = new DynamicHub();
    break;
  case TypeDynamicNode:
    building = new DynamicNode();
    break;
  case TypeFeedingTube:
    building = new FeedingTube();
    break;
  case TypeMultiwiniaZone:
    building = new MultiwiniaZone();
    break;
  case TypeChessBase:
    building = new ChessBase();
    break;
  case TypeChessPiece:
    building = new ChessPiece();
    break;
  case TypeCloneLab:
    building = new CloneLab();
    break;
  case TypeControlStation:
    building = new ControlStation();
    break;
  case TypePortal:
    building = new Portal();
    break;
  case TypeCrate:
    building = new Crate();
    break;
  case TypeStatue:
    building = new Statue();
    break;
  case TypeWallBuster:
    building = new WallBuster();
    break;
  case TypePulseBomb:
    building = new PulseBomb();
    break;
  case TypeRestrictionZone:
    building = new RestrictionZone();
    break;
  case TypeJumpPad:
    building = new JumpPad();
    break;
  case TypeAIObjective:
    building = new AIObjective();
    break;
  case TypeAIObjectiveMarker:
    building = new AIObjectiveMarker();
    break;
  case TypeEruptionMarker:
    building = new EruptionMarker();
    break;
  case TypeSmokeMarker:
    building = new SmokeMarker();
    break;
  }

  if (building)
  {
    if (_type == TypeRadarDish || _type == TypeControlTower || _type == TypeTrunkPort || _type == TypeIncubator || _type == TypeDynamicHub
      || _type == TypeFenceSwitch)
      building->m_isGlobal = true;

    building->m_type = _type;
  }

  return building;
}

int Building::GetTypeId(const char* _name)
{
  for (int i = 0; i < NumBuildingTypes; ++i)
  {
    if (stricmp(_name, GetTypeName(i)) == 0)
      return i;
  }
  return -1;
}

char* Building::GetTypeName(int _type)
{
  static char* buildingNames[] = {"Invalid", "Factory", // These must be in the
                                  "Cave", // Same order as defined
                                  "RadarDish", // in building.h
                                  "LaserFence", "ControlTower", "GunTurret", "Bridge", "Powerstation", "Tree", "Wall", "TrunkPort",
                                  "ResearchItem", "Library", "Generator", "Pylon", "PylonStart", "PylonEnd", "SolarPanel", "PowerSplitter",
                                  "TrackLink", "TrackJunction", "TrackStart", "TrackEnd", "Refinery", "Mine", "Yard", "DisplayScreen",
                                  "UpgradePort", "PrimaryUpgrade", "Incubator", "AntHill", "SafeArea", "Triffid", "SpiritReceiver",
                                  "ReceiverLink", "SpiritSpawner", "SpiritProcessor", "SpawnPoint", "SpawnPopulationLock",
                                  "SpawnPointMaster", "SpawnLink", "AITarget", "AISpawnPoint", "BlueprintStore", "BlueprintConsole",
                                  "BlueprintRelay", "ScriptTrigger", "Spam", "GodDish", "StaticShape", "FuelGenerator", "FuelPipe",
                                  "FuelStation", "EscapeRocket", "FenceSwitch", "DynamicHub", "DynamicNode", "FeedingTube",
                                  "MultiwiniaZone", "ChessPiece", "ChessBase", "CloneLab", "ControlStation", "Portal", "Crate", "Statue",
                                  "WallBuster", "PulseBomb", "RestrictionZone", "JumpPad", "AIObjective", "AIObjectiveMarker",
                                  "EruptionMarker", "SmokeMarker"};

  if (_type >= 0 && _type < NumBuildingTypes)
    return buildingNames[_type];
  DEBUG_ASSERT(false);
  return nullptr;
}

void Building::GetTypeNameTranslated(int _type, UnicodeString& _dest)
{
  char* typeName = GetTypeName(_type);

  char stringId[256];
  sprintf(stringId, "buildingname_%s", typeName);

  if (ISLANGUAGEPHRASE(stringId))
    _dest = LANGUAGEPHRASE(stringId);
  else
    _dest = UnicodeString(typeName);
}

void Building::ListSoundEvents(LList<char*>* _list)
{
  _list->PutData("Create");
  _list->PutData("Reprogramming"); // Remove me
  _list->PutData("ReprogramComplete");
  _list->PutData("ChangeTeam");
  _list->PutData("Damage");
}

char* Building::LogState(char* _message)
{
  static char s_result[1024];

  sprintf(s_result, "%sBUILDING Type[%s] Id[%d] pos[%5.2f,%5.2f,%5.2f], vel[%5.2f] front[%5.2f] centre[%5.2f], radius[%5.2f]",
          (_message) ? _message : "", GetTypeName(m_type), m_id.GetUniqueId(), m_pos.x, m_pos.y, m_pos.z, m_vel.x + m_vel.y + m_vel.z,
          m_front.x + m_front.y + m_front.z, m_centrePos.x + m_centrePos.y + m_centrePos.z, m_radius);

  return s_result;
}
