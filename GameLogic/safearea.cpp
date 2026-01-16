#include "pch.h"
#include "text_stream_readers.h"
#include "math_utils.h"
#include "text_renderer.h"
#include "language_table.h"
#include "safearea.h"
#include "app.h"
#include "location.h"
#include "team.h"
#include "entity_grid.h"
#include "gametimer.h"
#include "global_world.h"

SafeArea::SafeArea()
  : Building(),
    m_size(50.0),
    m_entitiesRequired(0),
    m_entityTypeRequired(Entity::TypeDarwinian),
    m_recountTimer(0.0),
    m_entitiesCounted(0) { m_type = TypeSafeArea; }

void SafeArea::Initialise(Building* _template)
{
  Building::Initialise(_template);

  m_size = static_cast<SafeArea*>(_template)->m_size;
  m_entitiesRequired = static_cast<SafeArea*>(_template)->m_entitiesRequired;
  m_entityTypeRequired = static_cast<SafeArea*>(_template)->m_entityTypeRequired;

  m_radius = m_size;
  m_centrePos = m_pos + Vector3(0, m_radius / 2, 0);
}

bool SafeArea::Advance()
{
  //
  // Is the world awake yet ?

  if (!g_app->m_location)
    return false;
  if (!g_app->m_location->m_teams)
    return false;
  if (g_app->m_location->m_teams[m_id.GetTeamId()]->m_teamType == TeamTypeUnused)
    return false;

  if (GetNetworkTime() > m_recountTimer)
  {
    int numFound;
    g_app->m_location->m_entityGrid->GetFriends(s_neighbours, m_pos.x, m_pos.z, m_size, &numFound, m_id.GetTeamId());
    m_entitiesCounted = 0;

    for (int i = 0; i < numFound; ++i)
    {
      WorldObjectId id = s_neighbours[i];
      Entity* entity = g_app->m_location->GetEntity(id);
      if (entity && entity->m_type == m_entityTypeRequired && !entity->m_dead)
        ++m_entitiesCounted;
    }

    m_recountTimer = GetNetworkTime() + 2.0;

    if ((m_id.GetTeamId() == 1 && m_entitiesCounted <= m_entitiesRequired) || (m_id.GetTeamId() != 1 && m_entitiesCounted >=
      m_entitiesRequired))
    {
      GlobalBuilding* gb = g_app->m_globalWorld->GetBuilding(m_id.GetUniqueId(), g_app->m_locationId);
      if (gb && !gb->m_online)
      {
        gb->m_online = true;
        g_app->m_globalWorld->EvaluateEvents();
      }
    }
  }

  return false;
}

bool SafeArea::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool SafeArea::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool SafeArea::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* _norm)
{
  return false;
}

void SafeArea::GetObjectiveCounter(UnicodeString& _dest)
{
  static wchar_t result[256];
  swprintf(result, sizeof(result) / sizeof(wchar_t), L"%ls : %d", LANGUAGEPHRASE("objective_currentcount").m_unicodestring,
           m_entitiesCounted);
  _dest = UnicodeString(result);
}

void SafeArea::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);

  m_size = atof(_in->GetNextToken());
  m_entitiesRequired = atoi(_in->GetNextToken());
  m_entityTypeRequired = atoi(_in->GetNextToken());
}

