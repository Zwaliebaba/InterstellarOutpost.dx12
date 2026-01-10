#include "pch.h"
#include "math_utils.h"
#include "resource.h"
#include "preferences.h"
#include "cheat_window.h"
#include "debugmenu.h"
#include "drop_down_menu.h"
#include "app.h"
#include "global_world.h"
#include "globals.h"
#include "location.h"
#include "unit.h"
#include "team.h"
#include "camera.h"
#include "renderer.h"
#include "taskmanager.h"
#include "markersystem.h"
#include "crate.h"

#ifdef CHEATMENU_ENABLED

class KillAllEnemiesButton : public DarwiniaButton
{
  void MouseUp() override
  {
    if (g_app->m_location)
    {
      int myTeamId = g_app->m_globalWorld->m_myTeamId;
      for (int t = 0; t < NUM_TEAMS; ++t)
      {
        if (!g_app->m_location->IsFriend(myTeamId, t))
        {
          Team* team = g_app->m_location->m_teams[t];

          // Kill all UNITS
          for (int u = 0; u < team->m_units.Size(); ++u)
          {
            if (team->m_units.ValidIndex(u))
            {
              Unit* unit = team->m_units[u];
              for (int e = 0; e < unit->m_entities.Size(); ++e)
              {
                if (unit->m_entities.ValidIndex(e))
                {
                  Entity* entity = unit->m_entities[e];
                  entity->ChangeHealth(entity->m_stats[Entity::StatHealth] * -2.0);
                }
              }
            }
          }

          // Kill all ENTITIES
          for (int o = 0; o < team->m_others.Size(); ++o)
          {
            if (team->m_others.ValidIndex(o))
            {
              Entity* entity = team->m_others[o];
              entity->ChangeHealth(entity->m_stats[Entity::StatHealth] * -2.0);
            }
          }
        }
      }

      for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
      {
        if (g_app->m_location->m_buildings.ValidIndex(i))
        {
          Building* building = g_app->m_location->m_buildings[i];
          if (building->m_type == Building::TypeAntHill || building->m_type == Building::TypeTriffid)
            building->Damage(-999);
        }
      }
    }
  }
};

class SpawnDarwiniansButton : public DarwiniaButton
{
  public:
    int m_teamId;

    void MouseUp() override
    {
      if (g_app->m_location)
      {
        Vector3 rayStart;
        Vector3 rayDir;
        g_app->m_camera->GetClickRay(g_app->m_renderer->ScreenW() / 2, g_app->m_renderer->ScreenH() / 2, &rayStart, &rayDir);
        Vector3 _pos;
        g_app->m_location->m_landscape.RayHit(rayStart, rayDir, &_pos);

        g_app->m_location->SpawnEntities(_pos, m_teamId, -1, Entity::TypeDarwinian, 20, g_zeroVector, 30);
      }
    }
};

class SpawnTankButton : public DarwiniaButton
{
  void MouseUp() override
  {
    if (g_app->m_location)
    {
      Vector3 rayStart;
      Vector3 rayDir;
      g_app->m_camera->GetClickRay(g_app->m_renderer->ScreenW() / 2, g_app->m_renderer->ScreenH() / 2, &rayStart, &rayDir);
      Vector3 _pos;
      g_app->m_location->m_landscape.RayHit(rayStart, rayDir, &_pos);

      g_app->m_location->SpawnEntities(_pos, 2, -1, Entity::TypeArmour, 1, g_zeroVector, 0);
    }
  }
};

class SpawnViriiButton : public DarwiniaButton
{
  void MouseUp() override
  {
    if (g_app->m_location)
    {
      Vector3 rayStart;
      Vector3 rayDir;
      g_app->m_camera->GetClickRay(g_app->m_renderer->ScreenW() / 2, g_app->m_renderer->ScreenH() / 2, &rayStart, &rayDir);
      Vector3 _pos;
      g_app->m_location->m_landscape.RayHit(rayStart, rayDir, &_pos);

      g_app->m_location->SpawnEntities(_pos, g_app->m_location->GetMonsterTeamId(), -1, Entity::TypeVirii, 20, g_zeroVector, 0, 1000.0);
    }
  }
};

class SpawnSpiritButton : public DarwiniaButton
{
  void MouseUp() override
  {
    if (g_app->m_location)
    {
      Vector3 rayStart;
      Vector3 rayDir;
      g_app->m_camera->GetClickRay(g_app->m_renderer->ScreenW() / 2, g_app->m_renderer->ScreenH() / 2, &rayStart, &rayDir);
      Vector3 _pos;
      g_app->m_location->m_landscape.RayHit(rayStart, rayDir, &_pos);

      for (int i = 0; i < 10; ++i)
      {
        Vector3 spiritPos = _pos + Vector3(SFRAND(20.0), 0.0, SFRAND(20.0));
        g_app->m_location->SpawnSpirit(spiritPos, g_zeroVector, 2, WorldObjectId());
      }
    }
  }
};

class AllowArbitraryPlacementButton : public DarwiniaButton
{
  void MouseUp() override
  {
    if (g_app->m_location)
      g_app->m_location->GetMyTaskManager()->m_verifyTargetting = false;
  }
};

class EnableGeneratorAndMineButton : public DarwiniaButton
{
  void MouseUp() override
  {
    int generatorLocationId = g_app->m_globalWorld->GetLocationId("generator");
    int mineLocationId = g_app->m_globalWorld->GetLocationId("mine");

    for (int i = 0; i < g_app->m_globalWorld->m_buildings.Size(); ++i)
    {
      if (g_app->m_globalWorld->m_buildings.ValidIndex(i))
      {
        GlobalBuilding* gb = g_app->m_globalWorld->m_buildings[i];
        if (gb && gb->m_locationId == generatorLocationId && gb->m_type == Building::TypeGenerator)
          gb->m_online = true;

        if (gb && gb->m_locationId == mineLocationId && gb->m_type == Building::TypeRefinery)
          gb->m_online = true;
      }
    }

    g_app->m_globalWorld->EvaluateEvents();
  }
};

class EnableReceiverAndBufferButton : public DarwiniaButton
{
  void MouseUp() override
  {
    int receiverLocationId = g_app->m_globalWorld->GetLocationId("receiver");
    int bufferLocationId = g_app->m_globalWorld->GetLocationId("pattern_buffer");

    for (int i = 0; i < g_app->m_globalWorld->m_buildings.Size(); ++i)
    {
      if (g_app->m_globalWorld->m_buildings.ValidIndex(i))
      {
        GlobalBuilding* gb = g_app->m_globalWorld->m_buildings[i];
        if (gb && gb->m_locationId == receiverLocationId && gb->m_type == Building::TypeSpiritProcessor)
          gb->m_online = true;

        if (gb && gb->m_locationId == bufferLocationId && gb->m_type == Building::TypeBlueprintStore)
          gb->m_online = true;
      }
    }

    g_app->m_globalWorld->EvaluateEvents();
  }
};

class OpenAllLocationsButton : public DarwiniaButton
{
  void MouseUp() override
  {
    for (int i = 0; i < g_app->m_globalWorld->m_locations.Size(); ++i)
    {
      GlobalLocation* loc = g_app->m_globalWorld->m_locations[i];
      loc->m_available = true;
    }
  }
};

class ClearResourcesButton : public DarwiniaButton
{
  void MouseUp() override
  {
    g_app->m_resource->FlushOpenGlState();
    g_app->m_resource->RegenerateOpenGlState();
  }
};

class GiveAllResearchButton : public DarwiniaButton
{
  void MouseUp() override
  {
    for (int i = 0; i < GlobalResearch::NumResearchItems; ++i)
    {
      g_app->m_globalWorld->m_research->AddResearch(i);
      if (g_app->IsSinglePlayer())
        g_app->m_globalWorld->m_research->SetCurrentProgress(i, 400);
      g_app->m_globalWorld->m_research->EvaluateLevel(i);
    }
  }
};

class SpawnPortsButton : public DarwiniaButton
{
  void MouseUp() override
  {
    for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
    {
      if (g_app->m_location->m_buildings.ValidIndex(i))
      {
        Building* building = g_app->m_location->m_buildings[i];
        for (int p = 0; p < building->GetNumPorts(); ++p)
        {
          Vector3 portPos, portFront;
          building->GetPortPosition(p, portPos, portFront);

          g_app->m_location->SpawnEntities(portPos, 0, -1, Entity::TypeDarwinian, 3, g_zeroVector, 30.0);
        }
      }
    }
  }
};

class ProfilerCreateButton : public DarwiniaButton
{
  void MouseUp() override { DebugKeyBindings::ProfileButton(); }
};

class NetworkStatsCreateButton : public DarwiniaButton
{
  void MouseUp() override { DebugKeyBindings::NetworkButton(); }
};

class CrateCreateButton : public DarwiniaButton
{
  public:
    bool m_good;
    int m_reward;

    CrateCreateButton()
      : DarwiniaButton(),
        m_good(true),
        m_reward(-1) {}

    void MouseUp() override
    {
      if (g_app->m_location)
      {
        Vector3 rayStart;
        Vector3 rayDir;
        g_app->m_camera->GetClickRay(g_app->m_renderer->ScreenW() / 2, g_app->m_renderer->ScreenH() / 2, &rayStart, &rayDir);
        Vector3 _pos;
        g_app->m_location->m_landscape.RayHit(rayStart, rayDir, &_pos);
        _pos.y = 300.0f;

        Crate crateTemplate;
        crateTemplate.m_pos = _pos;

        Building* newBuilding = Building::CreateBuilding(Building::TypeCrate);
        int id = g_app->m_location->m_buildings.PutData(newBuilding);
        newBuilding->Initialise(&crateTemplate);
        newBuilding->SetDetail(1);
        newBuilding->m_id.Set(255, UNIT_BUILDINGS, -1, g_app->m_globalWorld->GenerateBuildingId());
        static_cast<Crate*>(newBuilding)->m_rewardId = m_reward;

        g_app->m_markerSystem->RegisterMarker_Crate(newBuilding->m_id);
      }
    }
};

void CheatWindow::Create()
{
  DarwiniaWindow::Create();

  int y = 25;

  bool show = true;

  if (show)
  {
    auto killAllEnemies = new KillAllEnemiesButton();
    killAllEnemies->SetShortProperties("Kill All Enemies", 10, y, m_w - 20, -1, UnicodeString("Kill All Enemies"));
    RegisterButton(killAllEnemies);
    m_buttonOrder.PutData(killAllEnemies);

    auto spawnDarwiniansGreen = new SpawnDarwiniansButton();
    spawnDarwiniansGreen->SetShortProperties("Spawn Green", 10, y += 20, (m_w - 25) / 2, -1, UnicodeString("Spawn Green"));
    spawnDarwiniansGreen->m_teamId = 0;
    RegisterButton(spawnDarwiniansGreen);
    m_buttonOrder.PutData(spawnDarwiniansGreen);

    auto spawnDarwiniansRed = new SpawnDarwiniansButton();
    spawnDarwiniansRed->SetShortProperties("Spawn Red", spawnDarwiniansGreen->m_x + spawnDarwiniansGreen->m_w + 5, y, (m_w - 25) / 2, -1,
                                           UnicodeString("Spawn Red"));
    if (g_app->m_location)
      spawnDarwiniansRed->m_teamId = 1;
    RegisterButton(spawnDarwiniansRed);
    m_buttonOrder.PutData(spawnDarwiniansRed);

    auto spawnDarwiniansYellow = new SpawnDarwiniansButton();
    spawnDarwiniansYellow->SetShortProperties("Spawn Yellow", 10, y += 20, (m_w - 25) / 2, -1, UnicodeString("Spawn Yellow"));
    spawnDarwiniansYellow->m_teamId = 2;
    RegisterButton(spawnDarwiniansYellow);
    m_buttonOrder.PutData(spawnDarwiniansYellow);

    auto spawnDarwiniansblue = new SpawnDarwiniansButton();
    spawnDarwiniansblue->SetShortProperties("Spawn Blue", spawnDarwiniansGreen->m_x + spawnDarwiniansGreen->m_w + 5, y, (m_w - 25) / 2, -1,
                                            UnicodeString("Spawn Blue"));
    spawnDarwiniansblue->m_teamId = 3;
    RegisterButton(spawnDarwiniansblue);
    m_buttonOrder.PutData(spawnDarwiniansblue);

    auto spawnDarwiniansmonster = new SpawnDarwiniansButton();
    spawnDarwiniansmonster->SetShortProperties("Spawn Virus", 10, y += 20, (m_w - 25) / 2, -1, UnicodeString("Spawn Virus"));
    if (g_app->m_location)
      spawnDarwiniansmonster->m_teamId = g_app->m_location->GetMonsterTeamId();
    RegisterButton(spawnDarwiniansmonster);
    m_buttonOrder.PutData(spawnDarwiniansmonster);

    auto spawnDarwiniansfuture = new SpawnDarwiniansButton();
    spawnDarwiniansfuture->SetShortProperties("Spawn Future", spawnDarwiniansGreen->m_x + spawnDarwiniansGreen->m_w + 5, y, (m_w - 25) / 2,
                                              -1, UnicodeString("Spawn Future"));
    if (g_app->m_location)
      spawnDarwiniansfuture->m_teamId = g_app->m_location->GetFuturewinianTeamId();
    RegisterButton(spawnDarwiniansfuture);
    m_buttonOrder.PutData(spawnDarwiniansfuture);

    auto spawnTankButton = new SpawnTankButton();
    spawnTankButton->SetShortProperties("Spawn Armour", 10, y += 20, m_w - 20, -1, UnicodeString("Spawn Armour"));
    RegisterButton(spawnTankButton);
    m_buttonOrder.PutData(spawnTankButton);

    auto spawnVirii = new SpawnViriiButton();
    spawnVirii->SetShortProperties("Spawn Virii", 10, y += 20, m_w - 20, -1, UnicodeString("Spawn Virii"));
    RegisterButton(spawnVirii);
    m_buttonOrder.PutData(spawnVirii);

    auto spawnSpirits = new SpawnSpiritButton();
    spawnSpirits->SetShortProperties("Spawn Spirits", 10, y += 20, m_w - 20, -1, UnicodeString("Spawn Spirits"));
    RegisterButton(spawnSpirits);
    m_buttonOrder.PutData(spawnSpirits);

    auto allowPlacement = new AllowArbitraryPlacementButton();
    allowPlacement->SetShortProperties("Allow Arbitrary Placement", 10, y += 20, m_w - 20, -1, UnicodeString("Allow Arbitrary Placement"));
    RegisterButton(allowPlacement);
    m_buttonOrder.PutData(allowPlacement);

    auto enable = new EnableGeneratorAndMineButton();
    enable->SetShortProperties("Enable Generator and Mine", 10, y += 20, m_w - 20, -1, UnicodeString("Enable Generator and Mine"));
    RegisterButton(enable);
    m_buttonOrder.PutData(enable);

    auto receiver = new EnableReceiverAndBufferButton();
    receiver->SetShortProperties("Enable Receiver and Buffer", 10, y += 20, m_w - 20, -1, UnicodeString("Enable Receiver and Buffer"));
    RegisterButton(receiver);
    m_buttonOrder.PutData(receiver);

    auto openAllLocations = new OpenAllLocationsButton();
    openAllLocations->SetShortProperties("Open All Locations", 10, y += 20, m_w - 20, -1, UnicodeString("Open All Locations"));
    RegisterButton(openAllLocations);
    m_buttonOrder.PutData(openAllLocations);

    auto allResearch = new GiveAllResearchButton();
    allResearch->SetShortProperties("Give all research", 10, y += 20, m_w - 20, -1, UnicodeString("Give all research"));
    RegisterButton(allResearch);
    m_buttonOrder.PutData(allResearch);

    auto clearResources = new ClearResourcesButton();
    clearResources->SetShortProperties("Clear Resources", 10, y += 20, m_w - 20, -1, UnicodeString("Clear Resources"));
    RegisterButton(clearResources);
    m_buttonOrder.PutData(clearResources);

    auto spawnPorts = new SpawnPortsButton();
    spawnPorts->SetShortProperties("Spawn Ports", 10, y += 20, m_w - 20, -1, UnicodeString("Spawn Ports"));
    RegisterButton(spawnPorts);
    m_buttonOrder.PutData(spawnPorts);

    if (g_app->Multiplayer())
    {
      auto ccb = new CrateCreateButton();
      ccb->m_good = true;
      ccb->SetShortProperties("GoodCrate", 10, y += 20, (m_w / 2) - 20, -1, UnicodeString("GoodCrate"));
      RegisterButton(ccb);
      m_buttonOrder.PutData(ccb);

      auto crates = new DropDownMenu();
      crates->SetShortProperties("Crates", m_w / 2, y, (m_w / 2) - 10, -1, UnicodeString("Crates"));
      for (int p = 0; p < Crate::NumCrateRewards; ++p)
        crates->AddOption(Crate::GetName(p), p);
      RegisterButton(crates);
      m_buttonOrder.PutData(crates);
      crates->RegisterInt(&ccb->m_reward);
    }
    y += 20;
  }

  auto profiler = new ProfilerCreateButton();
  profiler->SetShortProperties("Profiler", 10, y += 20, m_w - 20, -1, UnicodeString("Profiler"));
  RegisterButton(profiler);
  m_buttonOrder.PutData(profiler);

  auto networkStats = new NetworkStatsCreateButton();
  networkStats->SetShortProperties("Network Stats", 10, y += 20, m_w - 20, -1, UnicodeString("Network Stats"));
  RegisterButton(networkStats);
  m_buttonOrder.PutData(networkStats);
}

#else   // CHEATMENU_ENABLED

void CheatWindow::Create() {}

#endif  // CHEATMENU_ENABLED

CheatWindow::CheatWindow(const char* _name)
  : DarwiniaWindow(_name) {}
