#include "pch.h"
#include "text_stream_readers.h"
#include "language_table.h"
#include "scripttrigger.h"
#include "app.h"
#include "location.h"
#include "entity_grid.h"
#include "script.h"
#include "camera.h"
#include "gametimer.h"

ScriptTrigger::ScriptTrigger()
  : Building(),
    m_range(100.0),
    m_entityType(SCRIPTRIGGER_RUNNEVER),
    m_linkId(-1),
    m_triggered(0),
    m_timer(0.0)
{
  m_type = TypeScriptTrigger;

  sprintf(m_scriptFilename, "NewScript");
}

void ScriptTrigger::Initialise(Building* _template)
{
  Building::Initialise(_template);

  auto trigger = static_cast<ScriptTrigger*>(_template);
  strcpy(m_scriptFilename, trigger->m_scriptFilename);
  m_range = trigger->m_range;
  m_entityType = trigger->m_entityType;
  m_linkId = trigger->m_linkId;
}

void ScriptTrigger::Trigger()
{
  if (strstr(m_scriptFilename, ".txt"))
  {
    // Run a script, speficied by filename
    g_app->m_script->RunScript(m_scriptFilename);
    m_triggered = -1;
  }
  else
    m_triggered = 1;
}

bool ScriptTrigger::Advance()
{
  if (m_entityType != SCRIPTRIGGER_RUNNEVER)
  {
    if (m_triggered == -1)
    {
      // Our script has been run, we are done
      return true;
    }
    if (m_triggered > 0)
    {
#ifdef USE_SEPULVEDA_HELP_TUTORIAL
      if (!g_app->m_sepulveda->IsTalking())
      {
        if (m_triggered == 1)
          g_app->m_sepulveda->HighlightBuilding(m_linkId, m_scriptFilename);
        char nextPhrase[256];
        sprintf(nextPhrase, "%s_%d", m_scriptFilename, m_triggered);
        if (!ISLANGUAGEPHRASE_ANY(nextPhrase))
        {
          // We've run out of strings to say, so we are now done
          g_app->m_sepulveda->ClearHighlights(m_scriptFilename);
          return true;
        }
        g_app->m_sepulveda->Say(nextPhrase);
        ++m_triggered;
      }
#else
      return true;
#endif
    }
    bool alreadyRunningScript = g_app->m_script->IsRunningScript() || !g_app->m_camera->IsInteractive();

    if (!alreadyRunningScript)
    {
      // We haven't been triggered yet
      m_timer -= SERVER_ADVANCE_PERIOD;

      if (m_timer <= 0.0)
      {
        m_timer = 1.0;

        if (m_entityType == SCRIPTRIGGER_RUNALWAYS)
          Trigger();
        else if (m_entityType == SCRIPTRIGGER_RUNCAMENTER)
        {
          double camDistance = (g_app->m_camera->GetPos() - m_pos).Mag();
          Vector3 camVel = g_app->m_camera->GetVel();
          bool camInteractive = g_app->m_camera->IsInteractive();

          if (camDistance <= m_range && camVel.Mag() < 5.0 && camInteractive)
            Trigger();
        }
        else if (m_entityType == SCRIPTRIGGER_RUNCAMVIEW)
        {
          double camDistance = (g_app->m_camera->GetPos() - m_pos).Mag();
          Vector3 camVel = g_app->m_camera->GetVel();
          bool camInteractive = g_app->m_camera->IsInteractive();
          bool inView = RaySphereIntersection(g_app->m_camera->GetPos(), g_app->m_camera->GetFront(), m_pos, m_range);

          if (camDistance <= (m_range + 300.0) && camVel.Mag() < 5.0 && camInteractive && inView)
            Trigger();

          if (camDistance <= m_range && camVel.Mag() < 5.0 && camInteractive)
            Trigger();
        }
        else
        {
          int numFound;
          int numCorrectTypeFound = 0;
          g_app->m_location->m_entityGrid->GetNeighbours(s_neighbours, m_pos.x, m_pos.z, m_range, &numFound);
          for (int i = 0; i < numFound; ++i)
          {
            WorldObjectId id = s_neighbours[i];
            if (id.IsValid() && id.GetTeamId() == m_id.GetTeamId())
            {
              Entity* entity = g_app->m_location->GetEntity(id);
              if (entity && entity->m_type == m_entityType)
              {
                ++numCorrectTypeFound;
                break;
              }
            }
          }

          if (numCorrectTypeFound > 0)
            Trigger();
        }
      }
    }
  }

  return Building::Advance();
}

bool ScriptTrigger::DoesSphereHit(const Vector3& _pos, double _radius) { return false; }

bool ScriptTrigger::DoesShapeHit(Shape* _shape, Matrix34 _transform) { return false; }

bool ScriptTrigger::DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* _norm)
{
  return false;
}

int ScriptTrigger::GetBuildingLink() { return m_linkId; }

void ScriptTrigger::SetBuildingLink(int _buildingId) { m_linkId = _buildingId; }

void ScriptTrigger::Read(TextReader* _in, bool _dynamic)
{
  Building::Read(_in, _dynamic);

  m_linkId = atoi(_in->GetNextToken());
  m_range = atof(_in->GetNextToken());

  strcpy(m_scriptFilename, _in->GetNextToken());

  char* entityType = _in->GetNextToken();
  if (stricmp(entityType, "always") == 0)
    m_entityType = SCRIPTRIGGER_RUNALWAYS;
  else if (stricmp(entityType, "never") == 0)
    m_entityType = SCRIPTRIGGER_RUNNEVER;
  else if (stricmp(entityType, "camenter") == 0)
    m_entityType = SCRIPTRIGGER_RUNCAMENTER;
  else if (stricmp(entityType, "camview") == 0)
    m_entityType = SCRIPTRIGGER_RUNCAMVIEW;
  else
    m_entityType = Entity::GetTypeId(entityType);
}
