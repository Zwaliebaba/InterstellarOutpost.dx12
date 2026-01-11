#include "pch.h"

#include "preferences.h"
#include "text_file_writer.h"
#include "text_stream_readers.h"
#include "random_number.h"
#include "app.h"
#include "entity_grid.h"
#include "location.h"
#include "main.h"
#include "team.h"
#include "obstruction_grid.h"
#include "multiwinia.h"
#include "aiobjective.h"
#include "building.h"
#include "ai.h"
#include "armour.h"
#include "rocket.h"

bool AIObjective::s_objectivesInitialised = false;

AIObjective::AIObjective()
  : Building(),
    m_nextObjective(-1),
    m_active(true),
    m_defenseObjective(false),
    m_timer(5.0),
    m_armourMarker(-1) { m_type = TypeAIObjective; }

void AIObjective::Initialise(Building* _template)
{
  Building::Initialise(_template);

  m_nextObjective = static_cast<AIObjective*>(_template)->m_nextObjective;
}

bool AIObjective::Advance()
{
  if (!s_objectivesInitialised)
    InitialiseObjectives();

  m_timer -= SERVER_ADVANCE_PERIOD;
  if (m_timer > 0.0)
    return false;

  m_timer = 5.0;

  if (m_defenseObjective)
    AdvanceDefensive();
  else
    AdvanceStandard();

  return false;
}

void AIObjective::AdvanceDefensive()
{
  if (!m_active)
    return;

  bool positionLost = false;
  for (int i = 0; i < m_objectiveMarkers.Size(); ++i)
  {
    Building* b = g_app->m_location->GetBuilding(m_objectiveMarkers[i]);
    if (b && b->m_type == TypeAIObjectiveMarker)
    {
      if (b->m_id.GetTeamId() != 255 && !g_app->m_location->IsDefending(b->m_id.GetTeamId()))
      {
        positionLost = true;
        break;
      }
    }
  }

  if (positionLost)
  {
    // we have lost this position, so fall back
    m_active = false;
    auto nextObjective = static_cast<AIObjective*>(g_app->m_location->GetBuilding(m_nextObjective));
    if (nextObjective)
      nextObjective->m_active = true;
  }
}

void AIObjective::AdvanceStandard()
{
  if (!m_active)
    return;
#ifdef TARGET_DEBUG
  DebugTrace("Updating Active Attack Objective\n");
#endif

  bool objectivesCaptured = true;
  for (int i = 0; i < m_objectiveMarkers.Size(); ++i)
  {
    Building* b = g_app->m_location->GetBuilding(m_objectiveMarkers[i]);
    if (b && b->m_type == TypeAIObjectiveMarker)
    {
      if (b->m_id.GetTeamId() == 255 || g_app->m_location->IsDefending(b->m_id.GetTeamId()))
      {
        if (static_cast<AIObjectiveMarker*>(b)->m_armourObjective == 0)
        {
          objectivesCaptured = false;
          break;
        }
      }
    }
  }

  if (objectivesCaptured)
  {
    DebugTrace("Objective Captured!\n");
    m_active = false;
    //SetTeamId( capturingTeam );
    auto nextObjective = static_cast<AIObjective*>(g_app->m_location->GetBuilding(m_nextObjective));
    if (nextObjective)
    {
      DebugTrace("Next Objective Activated\n");
      nextObjective->m_active = true;
    }

    for (int i = 0; i < m_objectiveMarkers.Size(); ++i)
    {
      auto m = static_cast<AIObjectiveMarker*>(g_app->m_location->GetBuilding(m_objectiveMarkers[i]));
      if (m && m->m_type == TypeAIObjectiveMarker)
      {
        if (m->m_armourObjective == 1 && m->m_pickupOnly != -1)
          m->m_pickupAvailable = true;
      }
    }
  }
}

void AIObjective::InitialiseObjectives()
{
  Armour::s_armourObjectives.Empty();
  for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
  {
    if (g_app->m_location->m_buildings.ValidIndex(i))
    {
      auto objective = static_cast<AIObjective*>(g_app->m_location->m_buildings[i]);
      if (objective->m_type == TypeAIObjective)
      {
        auto next = static_cast<AIObjective*>(g_app->m_location->GetBuilding(objective->m_nextObjective));
        if (next)
          next->m_active = false;

        if (objective->m_armourMarker != -1)
          Armour::s_armourObjectives.PutData(objective->m_id.GetUniqueId());
        if (g_app->m_location->IsDefending(objective->m_id.GetTeamId()))
          objective->m_defenseObjective = true;
        if (objective->m_defenseObjective && next)
          next->m_defenseObjective = true;
      }
    }
  }
  s_objectivesInitialised = true;
}

void AIObjective::RegisterObjectiveMarker(int _markerId, bool _armourMarker)
{
  m_objectiveMarkers.PutData(_markerId);
  if (_armourMarker)
    m_armourMarker = _markerId;
}

void AIObjective::SetBuildingLink(int _buildingId) { m_nextObjective = _buildingId; }

int AIObjective::GetBuildingLink() { return m_nextObjective; }

void AIObjective::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);
  m_nextObjective = atoi(_in->GetNextToken());
}

void AIObjective::Write(TextWriter* _out)
{
  Building::Write(_out);
  _out->printf("%4d", m_nextObjective);
}

bool AIObjective::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool AIObjective::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool AIObjective::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* _norm)
{
  return false;
}

AIObjectiveMarker::AIObjectiveMarker()
  : Building(),
    m_scanRange(100.0),
    m_timer(0.0),
    m_registered(false),
    m_pickupAvailable(false),
    m_defenseMarker(false),
    m_objectiveId(-1),
    m_armourObjective(0),
    m_pickupOnly(0),
    m_objectiveBuildingId(-1)
{
  m_type = TypeAIObjectiveMarker;
  m_timer = syncfrand(1.0);
}

void AIObjectiveMarker::Initialise(Building* _template)
{
  Building::Initialise(_template);

  m_scanRange = static_cast<AIObjectiveMarker*>(_template)->m_scanRange;
  m_objectiveId = static_cast<AIObjectiveMarker*>(_template)->m_objectiveId;
  m_armourObjective = static_cast<AIObjectiveMarker*>(_template)->m_armourObjective;
  m_pickupOnly = static_cast<AIObjectiveMarker*>(_template)->m_pickupOnly;
}

bool AIObjectiveMarker::Advance()
{
  if (!AIObjective::s_objectivesInitialised && g_app->m_multiwinia->m_gameType == Multiwinia::GameTypeAssault)
    return false;

  if (!m_registered && m_objectiveId != -1)
  {
    auto b = static_cast<AIObjective*>(g_app->m_location->GetBuilding(m_objectiveId));
    b->RegisterObjectiveMarker(m_id.GetUniqueId(), (m_armourObjective == 1 && m_pickupOnly != 1));
    m_registered = true;
    if (b->m_defenseObjective)
      m_defenseMarker = true;
    if (m_id.GetTeamId() != 255)
    {
      m_defaultTeam = m_id.GetTeamId();
      SetTeamId(255);
    }

    // see if there is a building present in the same area as the marker
    // if there is, assume that is the objective and check that instead of the number of darwinians
    for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
    {
      if (g_app->m_location->m_buildings.ValidIndex(i))
      {
        Building* b = g_app->m_location->m_buildings[i];
        if (b && (b->m_pos - m_pos).Mag() < 20.0)
        {
          if (b->m_type != TypeAIObjectiveMarker && b->m_type != TypeAITarget && b->m_type != TypeWall && b->m_type != TypeLaserFence && b->
            m_type != TypeStaticShape)
          {
            m_objectiveBuildingId = b->m_id.GetUniqueId();
            break;
          }
        }
      }
    }
  }

  // Only run this code sometimes
  m_timer -= SERVER_ADVANCE_PERIOD;
  if (m_timer > 0.0)
    return false;

  m_timer = 2.0;

  if (g_app->m_multiwinia->m_gameType == Multiwinia::GameTypeRocketRiot)
    AdvanceRocketRiot();
  else if (m_defenseMarker)
    AdvanceDefensive();
  else
    AdvanceStandard();

  return false;
}

void AIObjectiveMarker::AdvanceStandard()
{
  auto objective = static_cast<AIObjective*>(g_app->m_location->GetBuilding(m_objectiveId));
  if (objective && objective->m_active)
  {
    if (m_objectiveBuildingId != -1)
    {
      Building* b = g_app->m_location->GetBuilding(m_objectiveBuildingId);
      if (b && !b->m_destroyed)
      {
        if (b->m_type == TypeSolarPanel && b->GetNumPorts() == 0)
          m_objectiveBuildingId = -1;
        else
          SetTeamId(b->m_id.GetTeamId());
      }
      else
      {
        DebugTrace("Registered Objective Building missing\n");
        m_objectiveBuildingId = -1;
      }
    }

    if (m_objectiveBuildingId == -1)
    {
      if (m_scanRange == 0.0)
        SetTeamId(m_defaultTeam);
      else
      {
        bool include[NUM_TEAMS];
        memset(include, true, sizeof(bool) * NUM_TEAMS);
        include[g_app->m_location->GetMonsterTeamId()] = false;
        include[g_app->m_location->GetFuturewinianTeamId()] = false;
        int total = g_app->m_location->m_entityGrid->GetNumNeighbours(m_pos.x, m_pos.z, m_scanRange, include);
        if (total > 0)
        {
          int teamCounts[NUM_TEAMS];
          memset(teamCounts, 0, sizeof(int) * NUM_TEAMS);
          for (int i = 0; i < NUM_TEAMS; ++i)
          {
            if (i == g_app->m_location->GetMonsterTeamId())
              continue;
            if (i == g_app->m_location->GetFuturewinianTeamId())
              continue;
            teamCounts[i] = g_app->m_location->m_entityGrid->GetNumFriends(m_pos.x, m_pos.z, m_scanRange, i);
          }

          int currentWinner = 0;
          for (int i = 0; i < NUM_TEAMS; ++i)
          {
            if (teamCounts[i] > (teamCounts[currentWinner] * 4) && teamCounts[currentWinner] < 10)
              currentWinner = i;
          }

          SetTeamId(currentWinner);
        }
        else
          SetTeamId(255);
      }
    }
  }

  int id = AI::FindNearestTarget(m_pos);
  auto target = static_cast<AITarget*>(g_app->m_location->GetBuilding(id));
  if (target)
  {
    for (int i = 0; i < NUM_TEAMS; ++i)
    {
      if (g_app->m_location->IsAttacking(i))
      {
        AI* ai = AI::s_ai[i];
        if (ai)
        {
          if (ai->m_currentObjective == m_objectiveId)
          {
            if (m_armourObjective == 0)
            {
              if (!g_app->m_location->IsFriend(m_id.GetTeamId(), i))
              {
                target->m_priority[i] = 1.0;
                target->m_aiObjectiveTarget[i] = true;
              }
              else
                target->m_aiObjectiveTarget[i] = false;
            }
            else
              target->m_aiObjectiveTarget[i] = false;
          }
          else if (m_pickupAvailable)
          {
            target->m_priority[i] = 1.0;
            target->m_aiObjectiveTarget[i] = true;
          }
          else
            target->m_aiObjectiveTarget[i] = false;
        }
      }
    }
  }
}

void AIObjectiveMarker::AdvanceRocketRiot()
{
  int id = AI::FindNearestTarget(m_pos);
  auto target = static_cast<AITarget*>(g_app->m_location->GetBuilding(id));
  if (target)
  {
    int teamId = m_id.GetTeamId();
    if (teamId == 255)
      return;
    LobbyTeam* lobbyTeam = &g_app->m_multiwinia->m_teams[teamId];
    auto rocket = static_cast<EscapeRocket*>(g_app->m_location->GetBuilding(lobbyTeam->m_rocketId));

    if (rocket && rocket->m_state != EscapeRocket::StateLoading && target->m_id.GetTeamId() != teamId)
    {
      target->m_priority[teamId] = 1.0;
      target->m_aiObjectiveTarget[teamId] = true;
    }
    else
    {
      target->m_aiObjectiveTarget[teamId] = true;
      target->m_priority[teamId] = 0.0;
    }
  }
}

void AIObjectiveMarker::AdvanceDefensive()
{
  auto objective = static_cast<AIObjective*>(g_app->m_location->GetBuilding(m_objectiveId));
  if (objective && objective->m_active)
  {
    Building* b = g_app->m_location->GetBuilding(m_objectiveBuildingId);
    if (b)
    {
      if (b->m_type == TypeSolarPanel && b->GetNumPorts() == 0)
        m_objectiveBuildingId = -1;
    }
    else
      m_objectiveBuildingId = -1;

    int teamCounts[NUM_TEAMS];
    memset(teamCounts, 0, sizeof(int) * NUM_TEAMS);
    for (int i = 0; i < NUM_TEAMS; ++i)
    {
      if (i == g_app->m_location->GetMonsterTeamId())
        continue;
      if (i == g_app->m_location->GetFuturewinianTeamId())
        continue;
      teamCounts[i] = g_app->m_location->m_entityGrid->GetNumFriends(m_pos.x, m_pos.z, m_scanRange, i);
    }

    int currentWinner = 0;
    int totalDefenders = 0;
    for (int i = 0; i < NUM_TEAMS; ++i)
    {
      if (g_app->m_location->IsDefending(i))
      {
        totalDefenders += teamCounts[i];
        if (totalDefenders >= 15 || (b && g_app->m_location->IsDefending(b->m_id.GetTeamId())))
        {
          currentWinner = i;
          break;
        }
      }

      if (teamCounts[i] > (teamCounts[currentWinner] * 2))
        currentWinner = i;
    }

    SetTeamId(currentWinner);
  }

  int id = AI::FindNearestTarget(m_pos);
  auto target = static_cast<AITarget*>(g_app->m_location->GetBuilding(id));
  if (target)
  {
    for (int i = 0; i < NUM_TEAMS; ++i)
    {
      if (g_app->m_location->IsDefending(i))
      {
        AI* ai = AI::s_ai[i];
        if (ai)
        {
          if (ai->m_currentObjective == m_objectiveId)
          {
            if (g_app->m_location->IsFriend(m_id.GetTeamId(), i))
            {
              target->m_priority[i] = 1.0;
              target->m_aiObjectiveTarget[i] = true;
            }
            else
              target->m_aiObjectiveTarget[i] = false;
          }
          else
            target->m_aiObjectiveTarget[i] = false;
        }
      }
    }
  }
}

void AIObjectiveMarker::SetBuildingLink(int _buildingId) { m_objectiveId = _buildingId; }

int AIObjectiveMarker::GetBuildingLink() { return m_objectiveId; }

void AIObjectiveMarker::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);
  m_scanRange = atof(_in->GetNextToken());
  m_objectiveId = atoi(_in->GetNextToken());
  if (_in->TokenAvailable())
  {
    m_armourObjective = atoi(_in->GetNextToken());
    if (_in->TokenAvailable())
      m_pickupOnly = atoi(_in->GetNextToken());
  }
}

void AIObjectiveMarker::Write(TextWriter* _out)
{
  Building::Write(_out);
  _out->printf("%2.2f ", m_scanRange);
  _out->printf("%4d", m_objectiveId);
  _out->printf("%4d", m_armourObjective);
  _out->printf("%4d", m_pickupOnly);
}

bool AIObjectiveMarker::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool AIObjectiveMarker::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool AIObjectiveMarker::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* _norm)
{
  return false;
}
