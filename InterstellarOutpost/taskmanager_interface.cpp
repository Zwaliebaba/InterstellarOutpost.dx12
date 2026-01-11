#include "pch.h"
#include "text_renderer.h"
#include "math_utils.h"
#include "vector2.h"
#include "resource.h"
#include "bitmap.h"
#include "profiler.h"
#include "hi_res_time.h"
#include "language_table.h"

#include "ogl_extensions.h"
#include "binary_stream_readers.h"
#include "preferences.h"
#include "targetcursor.h"
#include "clienttoserver.h"
#include "app.h"
#include "global_world.h"
#include "location.h"
#include "taskmanager.h"
#include "taskmanager_interface.h"
#include "taskmanager_interface_icons.h"
#include "taskmanager_interface_multiwinia.h"
#include "team.h"
#include "entity_grid.h"
#include "camera.h"
#include "shaman_interface.h"
#include "soundsystem.h"
#include "insertion_squad.h"
#include "officer.h"
#include "darwinian.h"
#include "researchitem.h"
#include "trunkport.h"
#include "engineer.h"


ScreenZone::ScreenZone(char* _name, char* _tooltip, float _x, float _y, float _w, float _h, int _data)
  : m_x(_x),
    m_y(_y),
    m_w(_w),
    m_h(_h),
    m_data(_data),
    m_scrollZone(-1),
    m_subZones(false)
{
  ASSERT_TEXT(strlen(_name) < sizeof(m_name), "Button name too long : %s", _name);
  ASSERT_TEXT(strlen(_tooltip) < sizeof(m_toolTip), "Tooltip too long : %s", _tooltip);
  strcpy(m_name, _name);
  strcpy(m_toolTip, _tooltip);
}


KeyboardShortcut::KeyboardShortcut(std::string _name, int _data, ControlType _controltype)
  : ControlEventFunctor(_controltype),
    m_name(_name),
    m_data(_data) {}

const char* KeyboardShortcut::name() { return m_name.c_str(); }

int KeyboardShortcut::data() { return m_data; }


TaskManagerInterface::TaskManagerInterface()
  : m_visible(false),
    m_highlightedTaskId(-1),
    m_lockTaskManager(false),
    m_quickUnitVisible(false),
    m_showingRecentTask(false) {}

TaskManagerInterface::~TaskManagerInterface() {}

void TaskManagerInterface::CreateTaskManager()
{
  if (g_app->m_taskManagerInterface)
  {
    delete g_app->m_taskManagerInterface;
    g_app->m_taskManagerInterface = NULL;
  }

  InputMode _inputMode = g_inputManager->getInputMode();

  if (g_app->Multiplayer())
    g_app->m_taskManagerInterface = new TaskManagerInterfaceMultiwinia();
  else
    g_app->m_taskManagerInterface = new TaskManagerInterfaceIcons();
}

void TaskManagerInterface::SetCurrentMessage(int _messageType, int _taskType, float _timer)
{
  m_currentMessageType = _messageType;
  m_currentTaskType = _taskType;
  m_messageTimer = GetHighResTime() + _timer;
}

void TaskManagerInterface::RunDefaultObjective(GlobalEventCondition* _cond)
{
  switch (_cond->m_type)
  {
  case GlobalEventCondition::BuildingOnline:
  case GlobalEventCondition::BuildingOffline:
  {
    Building* building = g_app->m_location->GetBuilding(_cond->m_id);
    if (building)
    {
#ifdef USE_SEPULVEDA_HELP_TUTORIAL
      g_app->m_sepulveda->HighlightBuilding(building->m_id.GetUniqueId(), "RunDefaultObjective"); g_app->m_camera->
        RequestBuildingFocusMode(building, 250.0f, 75.0f); g_app->m_sepulveda->Say("objective_capturetrunk_1"); g_app->m_sepulveda->
        Say("objective_capturetrunk_2"); g_app->m_sepulveda->Say("objective_capturetrunk_3");
#endif
      m_viewingDefaultObjective = true;
    }
    break;
  }

  case GlobalEventCondition::ResearchOwned:
  {
    Building* building = NULL;
    for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
    {
      if (g_app->m_location->m_buildings.ValidIndex(i))
      {
        Building* thisBuilding = g_app->m_location->m_buildings[i];
        if (thisBuilding->m_type == Building::TypeResearchItem && static_cast<ResearchItem*>(thisBuilding)->m_researchType == _cond->m_id)
        {
          building = thisBuilding;
          break;
        }
      }
    }

    if (building)
    {
#ifdef USE_SEPULVEDA_HELP_TUTORIAL
      g_app->m_sepulveda->HighlightBuilding(building->m_id.GetUniqueId(), "RunDefaultObjective"); g_app->m_camera->
        RequestBuildingFocusMode(building, 100.0f, 75.0f); g_app->m_sepulveda->Say("objective_research_1"); g_app->m_sepulveda->Say(
        "objective_research_2");
#endif
      m_viewingDefaultObjective = true;
    }
    break;
  }
  }
}

void TaskManagerInterface::SetVisible(bool _visible) { m_visible = _visible; }

void TaskManagerInterface::AdvanceTab()
{
  if (!m_visible || g_inputManager->getInputMode() == INPUT_MODE_KEYBOARD)
  {
    int taskId = -1;
    int index = -1;
    bool changeTask = false;
    bool gesturesCycle = false;

    Team* myTeam = g_app->m_location->GetMyTeam();

    if (!myTeam)
      return;

    TaskManager* tm = myTeam->m_taskManager;

    if (g_inputManager->controlEvent(ControlGesturesSwitchUnit) && g_prefsManager->GetInt("ControlMethod", 0) == 0)
      gesturesCycle = true;

    if (g_inputManager->controlEvent(ControlUnitCycleRight) || gesturesCycle)
    {
      int taskToAimFor = tm->m_currentTaskId;
      if (taskToAimFor == -1)
        taskToAimFor = tm->m_previousTaskId;

      changeTask = true;
      for (int i = 0; i < tm->m_tasks.Size(); ++i)
      {
        if (tm->m_tasks.ValidIndex(i))
        {
          if (tm->m_tasks[i]->m_id == taskToAimFor)
          {
            if (tm->m_tasks.ValidIndex(i + 1))
              index = i + 1;
            else if (tm->m_tasks.ValidIndex(0))
              index = 0;
            break;
          }
        }
      }
    }

    if (g_inputManager->controlEvent(ControlUnitCycleLeft))
    {
      changeTask = true;
      for (int i = 0; i < tm->m_tasks.Size(); ++i)
      {
        if (tm->m_tasks.ValidIndex(i))
        {
          if (tm->m_tasks[i]->m_id == tm->m_currentTaskId)
          {
            if (tm->m_tasks.ValidIndex(i - 1))
              index = i - 1;
            else if (tm->m_tasks.ValidIndex(tm->m_tasks.Size() - 1))
              index = tm->m_tasks.Size() - 1;
            break;
          }
        }
      }
    }

    if (changeTask)
    {
      g_app->m_soundSystem->TriggerOtherEvent(NULL, "MouseOverIcon", SoundSourceBlueprint::TypeInterface);

      if (index == -1 && tm->m_tasks.ValidIndex(0))
        index = 0;

      if (tm->m_tasks.ValidIndex(index))
      {
        if (tm->m_tasks[index]->m_type == GlobalResearch::TypeSquad)
        {
          WorldObjectId id = tm->m_tasks[index]->m_objId;

          DebugTrace("Requesting tracking of unit: %d:%d:%d\n", id.GetTeamId(), id.GetUnitId(), id.GetIndex());
          g_app->m_camera->RequestEntityTrackMode(tm->m_tasks[index]->m_objId);
        }
        else
          g_app->m_camera->RequestMode(Camera::ModeFreeMovement);
        taskId = tm->m_tasks[index]->m_id;
        if (g_app->IsSinglePlayer())
          g_app->m_location->m_teams[g_app->m_globalWorld->m_myTeamId]->m_taskManager->SelectTask(taskId);
        else
          g_app->m_clientToServer->RequestSelectProgram(g_app->m_globalWorld->m_myTeamId, taskId);
      }
    }
  }
}

bool TaskManagerInterface::IsVisible() { return (m_visible || g_app->m_shamanInterface->IsVisible()); }
