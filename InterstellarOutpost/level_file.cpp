#include "pch.h"
#include "unicode_text_stream_reader.h"
#include "filesys_utils.h"
#include "resource.h"
#include "text_stream_readers.h"
#include "binary_stream_readers.h"
#include "random_number.h"
#include "filecrc.h"
#include "prefs_other_window.h"
#include "researchitem.h"
#include "darwinian.h"
#include "officer.h"
#include "anthill.h"
#include "incubator.h"
#include "armour.h"
#include "radardish.h"
#include "rocket.h"
#include "engineer.h"
#include "spawnpoint.h"
#include "ai.h"
#include "triffid.h"
#include "switch.h"
#include "laserfence.h"
#include "generichub.h"
#include "app.h"
#include "camera.h"
#include "global_world.h"
#include "level_file.h"
#include "location.h"
#include "routing_system.h"
#include "team.h"
#include "unit.h"
#include "taskmanager.h"
#include "mapfile.h"

//*****************************************************************************
// Class CamAnimNode
//*****************************************************************************

static const char* g_transitionModeNames[] = {"Move", "Cut"};

int CamAnimNode::GetTransitModeId(const char* _word)
{
  for (int i = 0; i < TransitionNumModes; ++i)
  {
    if (stricmp(_word, g_transitionModeNames[i]) == 0)
      return i;
  }

  return -1;
}

const char* CamAnimNode::GetTransitModeName(int _modeId)
{
  DEBUG_ASSERT(_modeId >= 0 && _modeId < TransitionNumModes);
  return g_transitionModeNames[_modeId];
}

//*****************************************************************************
// Class LevelFile
//*****************************************************************************

// ***************
// Private Methods
// ***************

void LevelFile::ParseMissionFile(const char* _filename)
{
  TextReader* in = NULL;
  char fullFilename[256];

  // Try to load a save game first
  sprintf(fullFilename, "%susers/%s/%s", g_app->GetProfileDirectory(), g_app->m_userProfileName, _filename);
  if (DoesFileExist(fullFilename))
    in = new UnicodeTextFileReader(fullFilename);

  if (in && !in->IsUnicode())
  {
    delete in;
    in = new TextFileReader(fullFilename);
  }

  if (!in)
  {
    sprintf(fullFilename, "levels/%s", _filename);
    in = g_app->m_resource->GetTextReader(fullFilename);
  }

  ASSERT_TEXT(in && in->IsOpen(), "Invalid level specified");

  while (in->ReadLine())
  {
    if (!in->TokenAvailable())
      continue;
    char* word = in->GetNextToken();

    if (stricmp("Landscape_StartDefinition", word) == 0 || stricmp("LandscapeTiles_StartDefinition", word) == 0 ||
      stricmp("LandFlattenAreas_StartDefinition", word) == 0 || stricmp("Lights_StartDefinition", word) == 0)
      DEBUG_ASSERT(0);
    else if (stricmp("CameraMounts_StartDefinition", word) == 0)
      ParseCameraMounts(in);
    else if (stricmp("CameraAnimations_StartDefinition", word) == 0)
      ParseCameraAnims(in);
    else if (stricmp("Buildings_StartDefinition", word) == 0)
      ParseBuildings(in, true);
    else if (stricmp("InstantUnits_StartDefinition", word) == 0)
      ParseInstantUnits(in);
    else if (stricmp("Routes_StartDefinition", word) == 0)
      ParseRoutes(in);
    else if (stricmp("PrimaryObjectives_StartDefinition", word) == 0)
      ParsePrimaryObjectives(in);
    else if (stricmp("RunningPrograms_StartDefinition", word) == 0)
      ParseRunningPrograms(in);
    else if (stricmp("Difficulty_StartDefinition", word) == 0)
      ParseDifficulty(in);
    else if (stricmp("LeaderboardStats_StartDefinition", word) == 0)
      ParseLeaderboardStats(in);
    else
    {
      // Looks like a damaged level file
      DEBUG_ASSERT(0);
    }
  }

  delete in;
}

void LevelFile::ParseMWMFile(const char* _filename)
{
  char fullfilename[512];
  sprintf(fullfilename, "%s%s", g_app->GetMapDirectory(), _filename);

  auto mapFile = new MapFile(fullfilename, true);
  TextReader* in = mapFile->GetTextReader();

  ParseMapFile(in);

  delete in;
  delete mapFile;
}

void LevelFile::ParseMapFile(TextReader* in)
{
  while (in->ReadLine())
  {
    if (!in->TokenAvailable())
      continue;
    char* word = in->GetNextToken();

    if (stricmp("MultiwiniaOptions_StartDefinition", word) == 0)
      ParseMultiwiniaOptions(in);
    else if (stricmp("landscape_startDefinition", word) == 0)
      ParseLandscapeData(in);
    else if (stricmp("landscapeTiles_startDefinition", word) == 0)
      ParseLandscapeTiles(in);
    else if (stricmp("landFlattenAreas_startDefinition", word) == 0)
      ParseLandFlattenAreas(in);
    else if (stricmp("Buildings_StartDefinition", word) == 0)
      ParseBuildings(in, false);
    else if (stricmp("Lights_StartDefinition", word) == 0)
      ParseLights(in);
    else if (stricmp("CameraMounts_StartDefinition", word) == 0)
      ParseCameraMounts(in);
    else if (stricmp("CameraAnimations_StartDefinition", word) == 0)
      ParseCameraAnims(in);
    else if (stricmp("Buildings_StartDefinition", word) == 0)
      ParseBuildings(in, true);
    else if (stricmp("InstantUnits_StartDefinition", word) == 0)
      ParseInstantUnits(in);
    else if (stricmp("Routes_StartDefinition", word) == 0)
      ParseRoutes(in);
    else if (stricmp("PrimaryObjectives_StartDefinition", word) == 0)
      ParsePrimaryObjectives(in);
    else
      DEBUG_ASSERT(0);
  }
}

void LevelFile::ParseMapFile(const char* _levelFilename)
{
  char fullFilename[256];
  sprintf(fullFilename, "levels/%s", _levelFilename);
  TextReader* in = g_app->m_resource->GetTextReader(fullFilename);
  if (!in)
  {
    sprintf(fullFilename, "%s%s", g_app->GetMapDirectory(), _levelFilename);
    in = new TextFileReader(fullFilename);
  }
  ASSERT_TEXT(in && in->IsOpen(), "Invalid map file specified\n");

  ParseMapFile(in);

  delete in;

}

void LevelFile::ParseCameraMounts(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("CameraMounts_EndDefinition", word) == 0)
      return;

    auto cmnt = new CameraMount();

    // Read name
    strncpy(cmnt->m_name, word, CAMERA_MOUNT_MAX_NAME_LEN);

    // Read pos
    word = _in->GetNextToken();
    cmnt->m_pos.x = atof(word);
    word = _in->GetNextToken();
    cmnt->m_pos.y = atof(word);
    word = _in->GetNextToken();
    cmnt->m_pos.z = atof(word);

    // Read front
    word = _in->GetNextToken();
    cmnt->m_front.x = atof(word);
    word = _in->GetNextToken();
    cmnt->m_front.y = atof(word);
    word = _in->GetNextToken();
    cmnt->m_front.z = atof(word);
    cmnt->m_front.Normalise();

    // Read up
    word = _in->GetNextToken();
    cmnt->m_up.x = atof(word);
    word = _in->GetNextToken();
    cmnt->m_up.y = atof(word);
    word = _in->GetNextToken();
    cmnt->m_up.z = atof(word);
    cmnt->m_up.Normalise();

    m_cameraMounts.PutData(cmnt);
  }
}

void LevelFile::ParseCameraAnims(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("CameraAnimations_EndDefinition", word) == 0)
      return;

    auto anim = new CameraAnimation();
    strncpy(anim->m_name, word, CAMERA_ANIM_MAX_NAME_LEN);

    while (_in->ReadLine())
    {
      if (!_in->TokenAvailable())
        continue;
      word = _in->GetNextToken();
      if (stricmp(word, "End") == 0)
        break;

      auto node = new CamAnimNode();

      // Read camera mode
      node->m_transitionMode = CamAnimNode::GetTransitModeId(word);
      ASSERT_TEXT(node->m_transitionMode >= 0 && node->m_transitionMode < Camera::ModeNumModes,
                  "Bad camera animation camera mode in level file %s", m_missionFilename);

      // Read mount name
      word = _in->GetNextToken();
      node->m_mountName = _strdup(word);
      if (stricmp(node->m_mountName, MAGIC_MOUNT_NAME_START_POS))
      {
        ASSERT_TEXT(GetCameraMount(node->m_mountName), "Bad camera animation mount name in level file %s", m_missionFilename);
      }

      // Read time
      word = _in->GetNextToken();
      node->m_duration = atof(word);
      ASSERT_TEXT(node->m_duration >= 0.0 && node->m_duration < 60.0, "Bad camera animation transition time in level file %s",
                  m_missionFilename);

      anim->m_nodes.PutDataAtEnd(node);
    }

    m_cameraAnimations.PutData(anim);
  }
}

void LevelFile::ParseLeaderboardStats(TextReader* _in)
{
  DebugTrace("Parsing LeaderboardStats_StartDefinition\n");
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("LeaderboardStats_EndDefinition", word) == 0)
    {
      DebugTrace("Done parsing leaderboardstats info\n");
      return;
    }
    if (stricmp("Time", word) == 0)
    {
      char* restOLine = _in->GetRestOfLine();
      DebugTrace("Rest of line is %s\n", restOLine);
      sscanf(restOLine, "%f", &m_leaderboardLevelTime);
      m_lastLeaderboardLevelTime = m_leaderboardLevelTime;
      DebugTrace("Time %f\n", m_leaderboardLevelTime);
    }
    else if (stricmp("Kills", word) == 0)
    {
      sscanf(_in->GetRestOfLine(), "%d", &(g_app->m_location->m_teams[2]->m_teamKills));
      DebugTrace("Kills %d\n", g_app->m_location->m_teams[2]->m_teamKills);
    }
    else if (stricmp("Survivors", word) == 0)
    {
      // Should not need survivors as that is in g_app->m_location->m_teams[0]->m_others.NumUsed();
      DebugTrace("Survivors\n");
    }
  }
}

void LevelFile::ParseMultiwiniaOptions(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("MultiwiniaOptions_EndDefinition", word) == 0)
      return;
    if (stricmp("PopulationCap", word) == 0)
    {
      int populationCap = atoi(_in->GetNextToken());
      m_populationCap = populationCap;
    }
    else if (stricmp("DefenderPopulationCap", word) == 0)
      m_defenderPopulationCap = atoi(_in->GetNextToken());
    else if (stricmp("Effects", word) == 0)
    {
      while (_in->TokenAvailable())
      {
        char* effect = _in->GetNextToken();
        if (strcmp(effect, "snow") == 0)
          m_effects[LevelEffectSnow] = true;
        if (strcmp(effect, "dustwind") == 0)
          m_effects[LevelEffectDustStorm] = true;
        if (strcmp(effect, "soulrain") == 0)
          m_effects[LevelEffectSoulRain] = true;
        if (strcmp(effect, "lightning") == 0)
          m_effects[LevelEffectLightning] = true;
        if (strcmp(effect, "SpecialLighting") == 0)
          m_effects[LevelEffectSpecialLighting] = true;
      }
    }
    else if (stricmp("GameTypes", word) == 0)
    {
      char* type = _in->GetNextToken();
      for (int i = 0; i < g_app->m_multiwinia->s_gameBlueprints.Size(); ++i)
      {
        if (strcmp(g_app->m_multiwinia->s_gameBlueprints[i]->m_name, type) == 0)
        {
          m_gameTypes = i;
          break;
        }
      }
    }
    else if (stricmp("NumPlayers", word) == 0)
      m_numPlayers = atoi(_in->GetNextToken());
    else if (stricmp("LevelOptions", word) == 0)
    {
      while (_in->TokenAvailable())
      {
        char* option = _in->GetNextToken();
        if (stricmp("nohandicap", option) == 0)
          m_levelOptions[LevelOptionNoHandicap] = 1;
        else if (stricmp("instantspawnpointcapture", option) == 0)
          m_levelOptions[LevelOptionInstantSpawnPointCapture] = 1;
        else if (stricmp("trunkportreinforcements", option) == 0)
          m_levelOptions[LevelOptionTrunkPortReinforcements] = atoi(_in->GetNextToken());
        else if (stricmp("trunkportarmour", option) == 0)
          m_levelOptions[LevelOptionTrunkPortArmour] = atoi(_in->GetNextToken());
        else if (stricmp("nogunturrets", option) == 0)
          m_levelOptions[LevelOptionNoGunTurrets] = 1;
        else if (stricmp("attackerreinforcements", option) == 0)
          m_levelOptions[LevelOptionAttackerReinforcements] = atoi(_in->GetNextToken());
        else if (stricmp("defenderreinforcements", option) == 0)
          m_levelOptions[LevelOptionDefenderReinforcements] = atoi(_in->GetNextToken());
        else if (stricmp("defenderreinforcementtype", option) == 0)
          m_levelOptions[LevelOptionDefenderReinforcementType] = GlobalResearch::GetType(_in->GetNextToken());
        else if (stricmp("attackerreinforcementtype", option) == 0)
          m_levelOptions[LevelOptionAttackerReinforcementType] = GlobalResearch::GetType(_in->GetNextToken());
        else if (stricmp("forceradarteammatch", option) == 0)
          m_levelOptions[LevelOptionForceRadarTeamMatch] = 1;
        else if (stricmp("noradarcontrol", option) == 0)
          m_levelOptions[LevelOptionNoRadarControl] = 1;
        else if (stricmp("airstrikecapturezones", option) == 0)
          m_levelOptions[LevelOptionAirstrikeCaptureZones] = atoi(_in->GetNextToken());
        else if (stricmp("futurewinianteam", option) == 0)
          m_levelOptions[LevelOptionFuturewinianTeam] = atoi(_in->GetNextToken());
        else if (stricmp("attackreinforcementoption", option) == 0)
          m_levelOptions[LevelOptionAttackReinforcementMode] = atoi(_in->GetNextToken());
        else if (stricmp("defendreinforcementoption", option) == 0)
          m_levelOptions[LevelOptionDefendReinforcementMode] = atoi(_in->GetNextToken());
      }
    }
    else if (stricmp("Name", word) == 0)
      strcpy(m_levelName, _in->GetRestOfLine());
    else if (stricmp("InvalidCrates", word) == 0)
    {
      while (_in->TokenAvailable())
      {
        char* crateType = _in->GetNextToken();
        for (int i = 0; i < Crate::NumCrateRewards; ++i)
        {
          if (stricmp(crateType, Crate::GetName(i)) == 0)
          {
            m_invalidCrates[i] = true;
            break;
          }
        }
      }
    }
    else if (stricmp("Author", word) == 0)
      strcpy(m_author, _in->GetRestOfLine());
    else if (stricmp("Description", word) == 0)
      strcpy(m_description, _in->GetRestOfLine());
    else if (stricmp("PlayTime", word) == 0)
      m_defaultPlayTime = atoi(_in->GetNextToken());
    else if (stricmp("Difficulty", word) == 0)
      m_difficultyRating = _strdup(_in->GetNextToken());
    else if (stricmp("AttackingTeams", word) == 0) { while (_in->TokenAvailable()) { m_attacking[atoi(_in->GetNextToken())] = true; } }
    else if (stricmp("CoopMode", word) == 0)
      m_coop = true;
    else if (stricmp("ForceCoop", word) == 0)
      m_forceCoop = true;
    else if (stricmp("CoopGroup1Positions", word) == 0)
    {
      while (_in->TokenAvailable()) { m_coopGroup1Positions.PutData(atoi(_in->GetNextToken())); }
    }
    else if (stricmp("CoopGroup2Positions", word) == 0)
    {
      while (_in->TokenAvailable()) { m_coopGroup2Positions.PutData(atoi(_in->GetNextToken())); }
    }
    else if (stricmp("OfficialLevel", word) == 0)
    {
      m_official = true;
      m_mapId = 0; // ### CRC based method here
    }
  }
}

void LevelFile::ParseBuildings(TextReader* _in, bool _dynamic)
{
  double loadDifficultyFactor = 1.0;
  if (m_levelDifficulty < 0)
    loadDifficultyFactor = 1.0 + static_cast<double>(g_app->m_difficultyLevel) / 5.0;

  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("Buildings_EndDefinition", word) == 0)
      return;

    Building* building = Building::CreateBuilding(word);
    if (building)
    {
      building->Read(_in, _dynamic);

      // Make sure that this building's ID isn't already in use
      int uniqueId = building->m_id.GetUniqueId();
      Building* existingBuilding = GetBuilding(uniqueId);
      if (existingBuilding)
      {
        ASSERT_TEXT(0, "%s UniqueId was not unique in %s", Building::GetTypeName(existingBuilding->m_type), _in->GetFilename());
      }

      // Make sure that it's global if it needs to be
      if (building->m_type == Building::TypeTrunkPort || building->m_type == Building::TypeControlTower || building->m_type ==
        Building::TypeRadarDish || building->m_type == Building::TypeIncubator || building->m_type == Building::TypeFenceSwitch)
      {
        ASSERT_TEXT(building->m_isGlobal, "Non-global %s found in %s", Building::GetTypeName(building->m_type), _in->GetFilename());
      }

      // Increase the difficulty by raising the population limits for the opposing forces
      if (building->m_id.GetTeamId() == 1)
      {
        switch (building->m_type)
        {
        case Building::TypeSpawnPopulationLock:
        {
          SpawnPopulationLock* spl = static_cast<SpawnPopulationLock*>(building);
          spl->m_maxPopulation = static_cast<int>(spl->m_maxPopulation * loadDifficultyFactor);
        }
        break;

        case Building::TypeAntHill:
        {
          AntHill* ah = static_cast<AntHill*>(building);
          ah->m_numAntsInside = static_cast<int>(ah->m_numAntsInside * loadDifficultyFactor);
        }
        break;

        case Building::TypeAISpawnPoint:
        {
          AISpawnPoint* aisp = static_cast<AISpawnPoint*>(building);
          aisp->m_period = static_cast<int>(aisp->m_period / loadDifficultyFactor);
        }
        break;

        case Building::TypeTriffid:
        {
          Triffid* t = static_cast<Triffid*>(building);
          t->m_reloadTime = static_cast<int>(t->m_reloadTime / loadDifficultyFactor);
        }
        break;
        }
      }

      m_buildings.PutData(building);
    }
  }
}

void LevelFile::ParseInstantUnits(TextReader* _in)
{
  double loadDifficultyFactor = 1.0;
  if (m_levelDifficulty < 0)
    loadDifficultyFactor = 1.0 + static_cast<double>(g_app->m_difficultyLevel) / 5.0;

  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("InstantUnits_EndDefinition", word) == 0)
      return;

    int entityType = Entity::GetTypeId(word);
    if (entityType == -1 || Entity::EntityIsBlocked(entityType))
      continue;

    auto iu = new InstantUnit();
    int numCopies = 0;
    iu->m_type = entityType;

    iu->m_teamId = atoi(_in->GetNextToken());
    iu->m_posX = atof(_in->GetNextToken());
    iu->m_posZ = atof(_in->GetNextToken());
    iu->m_number = atoi(_in->GetNextToken());
    iu->m_inAUnit = atoi(_in->GetNextToken()) ? true : false;
    iu->m_state = atoi(_in->GetNextToken());
    iu->m_spread = atof(_in->GetNextToken());

    if (_in->TokenAvailable())
    {
      iu->m_waypointX = atof(_in->GetNextToken());
      iu->m_waypointZ = atof(_in->GetNextToken());
    }

    if (_in->TokenAvailable())
    {
      iu->m_routeId = atoi(_in->GetNextToken());
      if (_in->TokenAvailable())
      {
        iu->m_routeWaypointId = atoi(_in->GetNextToken());
        if (_in->TokenAvailable())
          iu->m_runAsTask = atoi(_in->GetNextToken()) ? true : false;
      }
    }

    // Stop InstantSquaddies bug from crashing Darwinia (for existing bad save files)
    if (iu->m_type == Entity::TypeInsertionSquadie && iu->m_inAUnit == 0)
      continue;

    // If the unit is on the red team then make it more of them
    if (iu->m_teamId == 1)
    {
      iu->m_number = static_cast<int>(iu->m_number * loadDifficultyFactor);

      // It doesn't make sense to make overly 
      if (iu->m_type == Entity::TypeCentipede && loadDifficultyFactor > 1)
      {
        int maxCentipedeLength = 10 + static_cast<int>(2.0 * loadDifficultyFactor);
        int segmentUtilisation = maxCentipedeLength + 7;
        if (iu->m_number > maxCentipedeLength)
        {
          numCopies = iu->m_number / segmentUtilisation - 1;
          iu->m_number = maxCentipedeLength;
        }
      }
      else
      {
        if (loadDifficultyFactor > 1.0)
          iu->m_spread *= pow(1.2, g_app->m_difficultyLevel / 5.0);
      }
    }

    m_instantUnits.PutData(iu);

    // Create some additional centipedes if necessary
    for (int i = 0; i < numCopies; i++)
    {
      auto copy = new InstantUnit;
      *copy = *iu;
      // Spread them out a bit
      copy->m_posX = iu->m_posX + sfrand(60);
      copy->m_posZ = iu->m_posZ + sfrand(60);
      m_instantUnits.PutData(copy);
    }
  }
}

void LevelFile::ParseLandscapeData(TextReader* _in)
{
  while (_in->ReadLine())
  {
    char* word = _in->GetNextToken();
    char* secondWord = NULL;

    if (_in->TokenAvailable())
      secondWord = _in->GetNextToken();

    if (stricmp("cellSize", word) == 0)
      m_landscape.m_cellSize = atof(secondWord);
    else if (stricmp("worldSizeX", word) == 0)
      m_landscape.m_worldSizeX = atoi(secondWord);
    else if (stricmp("worldSizeZ", word) == 0)
      m_landscape.m_worldSizeZ = atoi(secondWord);
    else if (stricmp("outsideHeight", word) == 0)
      m_landscape.m_outsideHeight = atof(secondWord);
    else if (stricmp("maxHeight", word) == 0)
      m_landscape.m_maxHeight = atof(secondWord);
    else if (stricmp("landColourFile", word) == 0)
      strcpy(m_landscapeColourFilename, secondWord);
    else if (stricmp("wavesColourFile", word) == 0)
      strcpy(m_wavesColourFilename, secondWord);
    else if (stricmp("waterColourFile", word) == 0)
      strcpy(m_waterColourFilename, secondWord);
    else if (stricmp("landscape_endDefinition", word) == 0)
      return;
  }
}

void LevelFile::ParseLandscapeTiles(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp(word, "landscapeTiles_endDefinition") == 0)
      return;

    auto def = new LandscapeTile();
    m_landscape.m_tiles.PutDataAtEnd(def);

    def->m_posX = atoi(word);

    word = _in->GetNextToken();
    def->m_posY = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_posZ = atoi(word);

    word = _in->GetNextToken();
    def->m_size = atoi(word);

    word = _in->GetNextToken();
    def->m_fractalDimension = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_heightScale = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_desiredHeight = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_generationMethod = atoi(word);

    word = _in->GetNextToken();
    def->m_randomSeed = atoi(word);

    word = _in->GetNextToken();
    def->m_lowlandSmoothingFactor = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    int guidePower = atoi(word);
    def->GuideGridSetPower(guidePower);

    if (def->m_guideGridPower > 0)
    {
      word = _in->GetNextToken();
      def->GuideGridFromString(word);
    }
  }
}

void LevelFile::ParseLandFlattenAreas(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("landFlattenAreas_endDefinition", word) == 0)
      return;

    auto def = new LandscapeFlattenArea();
    m_landscape.m_flattenAreas.PutDataAtEnd(def);

    def->m_centre.x = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_centre.y = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_centre.z = static_cast<double>(atof(word));

    word = _in->GetNextToken();
    def->m_size = static_cast<double>(atof(word));

    if (_in->TokenAvailable())
    {
      def->m_flattenType = atoi(_in->GetNextToken());

      word = _in->GetNextToken();
      def->m_heightThreshold = static_cast<double>(atof(word));
    }
    else
      def->m_flattenType = LandscapeFlattenArea::FlattenTypeAbsolute;
  }
}

void LevelFile::ParseLights(TextReader* _in)
{
  bool ignoreLights = false;

  if (m_lights.Size() > 0)
  {
    // This function is called first when parsing the level file and 
    // secondly when parsing the map file. We only get here if the
    // level file specified some lights. In which case, these lights
    // are to be used in preference to the map lights. So we need
    // to ignore all the lights we encounter.
    ignoreLights = true;
  }

  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("Lights_EndDefinition", word) == 0)
      return;

    if (ignoreLights)
      continue;

    auto light = new Light;
    m_lights.PutData(light);

    light->m_front[0] = atof(word);

    word = _in->GetNextToken();
    light->m_front[1] = atof(word);

    word = _in->GetNextToken();
    light->m_front[2] = atof(word);
    light->m_front[3] = 0.0; // Set to be an infinitely distant light
    light->Normalise();

    word = _in->GetNextToken();
    light->m_colour[0] = atof(word);

    word = _in->GetNextToken();
    light->m_colour[1] = atof(word);

    word = _in->GetNextToken();
    light->m_colour[2] = atof(word);
    light->m_colour[3] = 0.0;
  }
}

void LevelFile::ParseRoute(TextReader* _in, int _id)
{
  auto r = new Route(_id);

  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("end", word) == 0)
      break;
    DEBUG_ASSERT(isdigit(word[0]));

    int type = atoi(word);
    Vector3 pos;
    int buildingId = -1;

    switch (type)
    {
    case WayPoint::TypeGroundPos:
      pos.x = atof(_in->GetNextToken());
      pos.z = atof(_in->GetNextToken());
      break;
    case WayPoint::Type3DPos:
      pos.x = atof(_in->GetNextToken());
      pos.y = atof(_in->GetNextToken());
      pos.z = atof(_in->GetNextToken());
      break;
    case WayPoint::TypeBuilding:
      buildingId = atoi(_in->GetNextToken());
      break;
    }

    auto wp = new WayPoint(type, pos);
    if (buildingId != -1)
      wp->m_buildingId = buildingId;
    r->m_wayPoints.PutDataAtEnd(wp);
  }

  m_routes.PutDataAtEnd(r);
}

void LevelFile::ParseRoutes(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("Routes_EndDefinition", word) == 0)
      return;

    if (stricmp("Route", word) == 0)
    {
      word = _in->GetNextToken();
      int id = atoi(word);
      DEBUG_ASSERT(id >= 0 && id < 10000);
      ParseRoute(_in, id);
    }
  }
}

void LevelFile::ParsePrimaryObjectives(TextReader* _in)
{
  while (_in->ReadLine())
  {
    char* word = _in->GetNextToken();

    if (stricmp("PrimaryObjectives_EndDefinition", word) == 0)
      return;

    auto condition = new GlobalEventCondition;
    condition->m_type = condition->GetType(word);
    DEBUG_ASSERT(condition->m_type != -1);

    switch (condition->m_type)
    {
    case GlobalEventCondition::AlwaysTrue:
    case GlobalEventCondition::NotInLocation:
      DEBUG_ASSERT(false);
      break;

    case GlobalEventCondition::BuildingOffline:
    case GlobalEventCondition::BuildingOnline:
    {
      //condition->m_locationId = g_app->m_globalWorld->GetLocationIdFromMapFilename( m_mapFilename );
      condition->m_locationId = g_app->m_globalWorld->GetLocationId(_in->GetNextToken());
      condition->m_id = atoi(_in->GetNextToken());
      DEBUG_ASSERT(condition->m_locationId != -1);
      break;
    }

    case GlobalEventCondition::ResearchOwned:
      condition->m_id = GlobalResearch::GetType(_in->GetNextToken());
      DEBUG_ASSERT(condition->m_id != -1);
      break;

    case GlobalEventCondition::DebugKey:
      condition->m_id = atoi(_in->GetNextToken());
      break;
    }

    if (_in->TokenAvailable())
    {
      char* stringId = _in->GetNextToken();
      condition->SetStringId(stringId);
    }

    if (_in->TokenAvailable())
    {
      char* cutScene = _in->GetNextToken();
      condition->SetCutScene(cutScene);
    }

    m_primaryObjectives.PutData(condition);
  }
}

void LevelFile::GenerateAutomaticObjectives()
{
  // 
  // Create a NeverTrue objective for cutscene mission files

  if (m_primaryObjectives.Size() == 0)
  {
    auto objective = new GlobalEventCondition;
    objective->m_type = GlobalEventCondition::NeverTrue;
    m_primaryObjectives.PutData(objective);
  }

  //
  // Add secondary objectives for trunk ports and research items

  for (int i = 0; i < m_buildings.Size(); ++i)
  {
    Building* building = m_buildings.GetData(i);

    if (building->m_type != Building::TypeResearchItem && building->m_type != Building::TypeTrunkPort)
      continue;

    // Make sure this building isn't already in the primary objectives list
    bool found = false;
    for (int k = 0; k < m_primaryObjectives.Size(); ++k)
    {
      GlobalEventCondition* primaryObjective = m_primaryObjectives.GetData(k);
      if (primaryObjective->m_id == building->m_id.GetUniqueId())
      {
        found = true;
        break;
      }

      if (primaryObjective->m_type == GlobalEventCondition::ResearchOwned && building->m_type == Building::TypeResearchItem && static_cast<
        ResearchItem*>(building)->m_researchType == primaryObjective->m_id)
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      int locationId = g_app->m_globalWorld->GetLocationIdFromMapFilename(m_mapFilename);

      if (building->m_type == Building::TypeResearchItem)
      {
        ResearchItem* item = static_cast<ResearchItem*>(GetBuilding(building->m_id.GetUniqueId()));
        int currentLevel = g_app->m_globalWorld->m_research->CurrentLevel(item->m_researchType);
        if (currentLevel == 0 /*|| currentLevel < item->m_level*/)
        {
          // NOTE : We SHOULD really allow objectives to be created when the current research level
          // is below that of this ResearchItem - eg we have level 1 and this item is level 2.
          // However GlobalEventCondition isn't currently able to store the level data, so it
          // ends up being an auto-completed objective.
          auto condition = new GlobalEventCondition();
          condition->m_locationId = locationId;
          condition->m_type = GlobalEventCondition::ResearchOwned;
          condition->m_id = item->m_researchType;
          condition->SetStringId("objective_research");
          m_secondaryObjectives.PutData(condition);
        }
      }
      else if (building->m_type == Building::TypeTrunkPort)
      {
        GlobalBuilding* gb = g_app->m_globalWorld->GetBuilding(building->m_id.GetUniqueId(), locationId);
        if (gb && !gb->m_online)
        {
          //
          // Is there a Control Tower that can enable this trunk port?
          bool towerFound = false;
          for (int c = 0; c < m_buildings.Size(); ++c)
          {
            Building* thisBuilding = m_buildings[c];
            if (thisBuilding->m_type == Building::TypeControlTower && thisBuilding->GetBuildingLink() == building->m_id.GetUniqueId())
            {
              towerFound = true;
              break;
            }
          }

          if (towerFound)
          {
            auto condition = new GlobalEventCondition;
            condition->m_locationId = locationId;
            condition->m_type = GlobalEventCondition::BuildingOnline;
            condition->m_id = building->m_id.GetUniqueId();
            condition->SetStringId("objective_capture_trunk");
            m_secondaryObjectives.PutData(condition);
          }
        }
      }
    }
  }
}

// **************
// Public Methods
// **************

LevelFile::LevelFile()
{
  sprintf(m_landscapeColourFilename, "landscape_default.bmp");
  sprintf(m_wavesColourFilename, "waves_default.bmp");
  sprintf(m_waterColourFilename, "water_default.bmp");
  m_levelDifficulty = -1;
  m_populationCap = -1;
  m_defenderPopulationCap = -1;
  m_numPlayers = -1;
  m_defaultPlayTime = -1;
  m_coop = false;
  m_forceCoop = false;
  m_official = false;
  m_mapId = -1;
  m_leaderboardLevelTime = 0.0;
  m_lastLeaderboardLevelTime = 9999999.0;
  m_gameTypes = -1;

  // m_author = NULL;
  //m_description = NULL;
  m_difficultyRating = NULL;

  memset(m_levelOptions, -1, sizeof(int) * NumLevelOptions);
  memset(m_invalidCrates, false, sizeof(bool) * Crate::NumCrateRewards);
  memset(m_attacking, false, sizeof(bool) * NUM_TEAMS);
  memset(m_effects, false, sizeof(m_effects));
  strcpy(m_levelName, "NewLevel");
  strcpy(m_description, "LevelDescription");
  strcpy(m_author, "Author");
  strcpy(m_thumbnailFilename, "default.bmp");

  m_levelOptions[LevelOptionNoHandicap] = 0;
  m_levelOptions[LevelOptionInstantSpawnPointCapture] = 0;
  m_levelOptions[LevelOptionNoGunTurrets] = 0;
  m_levelOptions[LevelOptionForceRadarTeamMatch] = 0;
  m_levelOptions[LevelOptionNoRadarControl] = 0;
}

LevelFile::LevelFile(const char* _missionFilename, const char* _mapFilename, bool _textFileLevel)
{
  sprintf(m_mapFilename, "%s", _mapFilename);
  sprintf(m_missionFilename, "%s", _missionFilename);
  sprintf(m_landscapeColourFilename, "landscape_default.bmp");
  sprintf(m_wavesColourFilename, "waves_default.bmp");
  sprintf(m_waterColourFilename, "water_default.bmp");
  m_levelDifficulty = -1;
  m_populationCap = -1;
  m_defenderPopulationCap = -1;
  m_numPlayers = -1;
  m_defaultPlayTime = -1;
  m_coop = false;
  m_forceCoop = false;
  m_official = false;
  m_mapId = -1;
  m_leaderboardLevelTime = 0.0;
  m_lastLeaderboardLevelTime = 9999999.0;
  m_gameTypes = -1;

  //m_author = NULL;
  //m_description = NULL;
  m_difficultyRating = NULL;

  memset(m_levelOptions, -1, sizeof(int) * NumLevelOptions);
  memset(m_invalidCrates, false, sizeof(bool) * Crate::NumCrateRewards);
  memset(m_attacking, false, sizeof(bool) * NUM_TEAMS);
  memset(m_effects, false, sizeof(m_effects));
  strcpy(m_levelName, "NewLevel");
  strcpy(m_description, "LevelDescription");
  strcpy(m_author, "Author");
  strcpy(m_thumbnailFilename, "default.bmp");

  m_levelOptions[LevelOptionNoHandicap] = 0;
  m_levelOptions[LevelOptionInstantSpawnPointCapture] = 0;
  m_levelOptions[LevelOptionNoGunTurrets] = 0;
  m_levelOptions[LevelOptionForceRadarTeamMatch] = 0;
  m_levelOptions[LevelOptionNoRadarControl] = 0;

  // Make sure that the current game difficulty setting
  // is consistent with the preferences (it can become inconsistent
  // when a level is loaded that was saved with a different difficulty
  // level to what the preferences say).	
  g_app->UpdateDifficultyFromPreferences();

  if (_textFileLevel)
  {
    if (stricmp(_missionFilename, "null") != 0)
      ParseMissionFile(m_missionFilename);
    ParseMapFile(m_mapFilename);
  }
  else
    ParseMWMFile(m_mapFilename);

  GenerateAutomaticObjectives();
}

LevelFile::~LevelFile()
{
  m_cameraMounts.EmptyAndDelete();
  m_cameraAnimations.EmptyAndDelete();
  m_buildings.EmptyAndDelete();
  m_instantUnits.EmptyAndDelete();
  m_lights.EmptyAndDelete();
  m_routes.EmptyAndDelete();
  m_runningPrograms.EmptyAndDelete();
  m_primaryObjectives.EmptyAndDelete();
  m_secondaryObjectives.EmptyAndDelete();
}

Building* LevelFile::GetBuilding(int _id)
{
  for (int i = 0; i < m_buildings.Size(); ++i)
  {
    Building* building = m_buildings.GetData(i);
    if (building->m_id.GetUniqueId() == _id)
      return building;
  }
  return NULL;
}

CameraMount* LevelFile::GetCameraMount(const char* _name)
{
  for (int i = 0; i < m_cameraMounts.Size(); ++i)
  {
    CameraMount* mount = m_cameraMounts.GetData(i);
    if (stricmp(mount->m_name, _name) == 0)
      return mount;
  }
  return NULL;
}

int LevelFile::GetCameraAnimId(const char* _name)
{
  for (int i = 0; i < m_cameraAnimations.Size(); ++i)
  {
    CameraAnimation* anim = m_cameraAnimations.GetData(i);
    if (stricmp(anim->m_name, _name) == 0)
      return i;
  }
  return -1;
}

void LevelFile::RemoveBuilding(int _id)
{
  for (int i = 0; i < m_buildings.Size(); ++i)
  {
    if (m_buildings.ValidIndex(i))
    {
      Building* building = m_buildings.GetData(i);
      if (building->m_id.GetUniqueId() == _id)
      {
        m_buildings.RemoveData(i);
        delete building;
        break;
      }
    }
  }
}

int LevelFile::GenerateNewRouteId()
{
  for (int i = 0; i < m_routes.Size(); ++i)
  {
    bool idNotUsed = true;
    for (int j = 0; j < m_routes.Size(); ++j)
    {
      Route* r = m_routes.GetData(j);
      if (i == r->m_id)
      {
        idNotUsed = false;
        break;
      }
    }

    if (idNotUsed)
      return i;
  }

  return m_routes.Size();
}

Route* LevelFile::GetRoute(int _id)
{
  int size = m_routes.Size();
  for (int i = 0; i < size; ++i)
  {
    Route* route = m_routes.GetData(i);
    if (route->m_id == _id)
      return route;
  }

  return NULL;
}

void LevelFile::GenerateInstantUnits()
{
  m_instantUnits.EmptyAndDelete();

  //
  // Record all the full size UNITS that exist

  for (int t = 0; t < NUM_TEAMS; ++t)
  {
    Team* team = g_app->m_location->m_teams[t];
    if (team->m_teamType == TeamTypeCPU)
    {
      for (int u = 0; u < team->m_units.Size(); ++u)
      {
        if (team->m_units.ValidIndex(u))
        {
          Unit* unit = team->m_units[u];

          Vector3 centrePos;
          double roamRange = 0;
          int numFound = 0;
          for (int i = 0; i < unit->m_entities.Size(); ++i)
          {
            if (unit->m_entities.ValidIndex(i))
            {
              Entity* entity = unit->m_entities[i];
              centrePos += entity->m_spawnPoint;
              roamRange += entity->m_roamRange;
              numFound++;
            }
          }

          centrePos /= static_cast<double>(numFound);
          roamRange /= static_cast<double>(numFound);

          auto instant = new InstantUnit();
          instant->m_type = unit->m_troopType;
          instant->m_teamId = unit->m_teamId;
          instant->m_posX = centrePos.x;
          instant->m_posZ = centrePos.z;
          instant->m_number = numFound;
          instant->m_inAUnit = true;
          instant->m_spread = roamRange;
          instant->m_routeId = unit->m_routeId;
          instant->m_routeWaypointId = unit->m_routeWayPointId;
          m_instantUnits.PutData(instant);
        }
      }
    }
  }

  //
  // Record all other entities that exist
  // If they are in transit in a teleport, ignore them
  // (dealt with later)

  for (int t = 0; t < NUM_TEAMS; ++t)
  {
    Team* team = g_app->m_location->m_teams[t];
    if (team->m_teamType == TeamTypeCPU)
    {
      for (int i = 0; i < team->m_others.Size(); ++i)
      {
        if (team->m_others.ValidIndex(i))
        {
          Entity* entity = team->m_others[i];
          if (entity->m_enabled)
          {
            bool insideSpawnArea = (entity->m_pos - entity->m_spawnPoint).Mag() < entity->m_roamRange;

            auto unit = new InstantUnit();
            unit->m_type = entity->m_type;
            unit->m_teamId = t;
            unit->m_posX = insideSpawnArea ? entity->m_spawnPoint.x : entity->m_pos.x;
            unit->m_posZ = insideSpawnArea ? entity->m_spawnPoint.z : entity->m_pos.z;
            unit->m_spread = insideSpawnArea ? entity->m_roamRange : 0;
            unit->m_number = 1;
            unit->m_inAUnit = false;
            unit->m_routeId = entity->m_routeId;
            unit->m_routeWaypointId = entity->m_routeWayPointId;

            if (entity->m_type == Entity::TypeDarwinian)
            {
              Darwinian* darwinian = static_cast<Darwinian*>(entity);
              unit->m_posX = darwinian->m_pos.x;
              unit->m_posZ = darwinian->m_pos.z;
              unit->m_waypointX = darwinian->m_wayPoint.x;
              unit->m_waypointZ = darwinian->m_wayPoint.z;
              unit->m_spread = 0.0; // Darwinians should be placed exactly where they were when the game was saved
              if (darwinian->m_state == Darwinian::StateFollowingOrders)
                unit->m_state = Darwinian::StateFollowingOrders;
            }

            m_instantUnits.PutData(unit);
          }
        }
      }
    }
    if (team->m_teamType == TeamTypeLocalPlayer)
    {
      for (int i = 0; i < team->m_others.Size(); ++i)
      {
        if (team->m_others.ValidIndex(i))
        {
          Entity* entity = team->m_others[i];
          if (entity->m_type == Entity::TypeOfficer && entity->m_enabled)
          {
            Officer* officer = static_cast<Officer*>(entity);
            auto unit = new InstantUnit();
            unit->m_type = entity->m_type;
            unit->m_teamId = t;
            unit->m_posX = entity->m_pos.x;
            unit->m_posZ = entity->m_pos.z;
            unit->m_spread = 0;
            unit->m_number = 1;
            unit->m_inAUnit = false;
            unit->m_state = officer->m_orders;
            unit->m_waypointX = officer->m_orderPosition.x;
            unit->m_waypointZ = officer->m_orderPosition.z;
            unit->m_routeId = officer->m_routeId;
            unit->m_routeWaypointId = officer->m_routeWayPointId;
            m_instantUnits.PutData(unit);
          }
          else if (entity->m_type == Entity::TypeArmour)
          {
            bool taskControlled = false;
            for (int i = 0; i < g_app->m_location->m_teams[t]->m_taskManager->m_tasks.Size(); ++i)
            {
              Task* task = g_app->m_location->m_teams[t]->m_taskManager->m_tasks[i];
              if (task->m_type == GlobalResearch::TypeArmour && task->m_objId == entity->m_id)
              {
                taskControlled = true;
                break;
              }
            }
            if (!taskControlled)
            {
              Armour* armour = static_cast<Armour*>(entity);
              auto unit = new InstantUnit();
              unit->m_type = Entity::TypeArmour;
              unit->m_teamId = t;
              unit->m_posX = armour->m_pos.x;
              unit->m_posZ = armour->m_pos.z;
              unit->m_spread = 0;
              unit->m_number = 1;
              unit->m_inAUnit = false;
              unit->m_state = armour->m_state;
              unit->m_waypointX = armour->m_wayPoint.x;
              unit->m_waypointZ = armour->m_wayPoint.z;
              unit->m_routeId = armour->m_routeId;
              unit->m_routeWaypointId = armour->m_routeWayPointId;
              m_instantUnits.PutData(unit);
            }
          }
        }
      }
    }
  }

  //
  // Record all entities in transit in a Radar Dish beam

  for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
  {
    if (g_app->m_location->m_buildings.ValidIndex(i))
    {
      Building* building = g_app->m_location->m_buildings[i];
      if (building && building->m_type == Building::TypeRadarDish)
      {
        RadarDish* dish = static_cast<RadarDish*>(building);
        Vector3 exitPos, exitFront;
        dish->GetExit(exitPos, exitFront);

        for (int e = 0; e < dish->m_inTransit.Size(); ++e)
        {
          WorldObjectId id = *dish->m_inTransit.GetPointer(e);
          Entity* entity = g_app->m_location->GetEntity(id);

          if (entity == NULL)
            continue;

          if (entity->m_type == Entity::TypeInsertionSquadie)
          {
            // InsertionSquaddies are running programs and will be saved there, so no
            // need to create an InstantUnit for them. However, we do need to adjust the
            // position of the squaddies so that they are not dunked in the water when revived
            entity->m_pos.x = exitPos.x;
            entity->m_pos.z = exitPos.z;
            entity->m_pos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(entity->m_pos.x, entity->m_pos.z) + 0.1;
          }
          else
          {
            auto unit = new InstantUnit();
            unit->m_type = entity->m_type;
            unit->m_teamId = id.GetTeamId();
            unit->m_posX = exitPos.x;
            unit->m_posZ = exitPos.z;
            unit->m_spread = 50;
            unit->m_number = 1;
            unit->m_inAUnit = false;
            unit->m_state = 0;
            unit->m_routeId = entity->m_routeId;
            unit->m_routeWaypointId = entity->m_routeWayPointId;
            //unit->m_waypointX = officer->m_orderPosition.x;
            //unit->m_waypointZ = officer->m_orderPosition.z;
            m_instantUnits.PutData(unit);
          }
        }
      }
    }
  }

}

void LevelFile::GenerateDynamicBuildings()
{
  //
  // Update buildings if they are dynamic
  // Remove all dynamic buildings from the list
  // that aren't on the level anymore

  for (int i = 0; i < m_buildings.Size(); ++i)
  {
    Building* building = m_buildings[i];
    if (building && building->m_dynamic)
    {
      Building* locBuilding = g_app->m_location->GetBuilding(building->m_id.GetUniqueId());
      if (!locBuilding)
      {
        m_buildings.RemoveData(i);
        delete building;
        --i;
      }
      else
      {
        if (building->m_type == Building::TypeAntHill)
          static_cast<AntHill*>(building)->m_numAntsInside = static_cast<AntHill*>(locBuilding)->m_numAntsInside;
        else if (building->m_type == Building::TypeIncubator)
          static_cast<Incubator*>(building)->m_numStartingSpirits = static_cast<Incubator*>(locBuilding)->NumSpiritsInside();
        else if (building->m_type == Building::TypeEscapeRocket)
        {
          static_cast<EscapeRocket*>(building)->m_fuel = static_cast<EscapeRocket*>(locBuilding)->m_fuel;
          static_cast<EscapeRocket*>(building)->m_passengers = static_cast<EscapeRocket*>(locBuilding)->m_passengers;
          static_cast<EscapeRocket*>(building)->m_spawnCompleted = static_cast<EscapeRocket*>(locBuilding)->m_spawnCompleted;
        }
        else if (building->m_type == Building::TypeFenceSwitch)
        {
          static_cast<FenceSwitch*>(building)->m_locked = static_cast<FenceSwitch*>(locBuilding)->m_locked;
          static_cast<FenceSwitch*>(building)->m_switchValue = static_cast<FenceSwitch*>(locBuilding)->m_switchValue;
        }
        else if (building->m_type == Building::TypeLaserFence)
          static_cast<LaserFence*>(building)->m_mode = static_cast<LaserFence*>(locBuilding)->m_mode;
        else if (building->m_type == Building::TypeDynamicHub)
          static_cast<DynamicHub*>(building)->m_currentScore = static_cast<DynamicHub*>(locBuilding)->m_currentScore;
        else if (building->m_type == Building::TypeDynamicNode)
          static_cast<DynamicNode*>(building)->m_scoreSupplied = static_cast<DynamicNode*>(locBuilding)->m_scoreSupplied;
        else if (building->m_type == Building::TypeAISpawnPoint)
          static_cast<AISpawnPoint*>(building)->m_spawnLimit = static_cast<AISpawnPoint*>(locBuilding)->m_spawnLimit;
      }
    }
  }

  //
  // Search for new dynamic buildings on the level

  for (int i = 0; i < g_app->m_location->m_buildings.Size(); ++i)
  {
    if (g_app->m_location->m_buildings.ValidIndex(i))
    {
      Building* building = g_app->m_location->m_buildings[i];
      if (building && building->m_dynamic)
      {
        Building* levelFileBuilding = GetBuilding(building->m_id.GetUniqueId());
        if (!levelFileBuilding)
        {
          Building* newBuilding = Building::CreateBuilding(building->m_type);
          newBuilding->m_id = building->m_id;
          newBuilding->m_pos = building->m_pos;
          newBuilding->m_front = building->m_front;
          newBuilding->m_type = building->m_type;
          newBuilding->m_dynamic = building->m_dynamic;
          newBuilding->m_isGlobal = building->m_isGlobal;
          m_buildings.PutData(newBuilding);
        }
      }
    }
  }
}

void LevelFile::ParseRunningPrograms(TextReader* _in)
{
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("RunningPrograms_EndDefinition", word) == 0)
      return;

    auto program = new RunningProgram;
    program->m_type = Entity::GetTypeId(word);
    program->m_count = atoi(_in->GetNextToken());
    program->m_state = atoi(_in->GetNextToken());
    program->m_data = atoi(_in->GetNextToken());
    program->m_waypointX = atof(_in->GetNextToken());
    program->m_waypointZ = atof(_in->GetNextToken());

    for (int i = 0; i < program->m_count; ++i)
    {
      program->m_positionX[i] = atof(_in->GetNextToken());
      program->m_positionZ[i] = atof(_in->GetNextToken());
      program->m_health[i] = atoi(_in->GetNextToken());
    }

    m_runningPrograms.PutData(program);
  }
}

void LevelFile::ParseDifficulty(TextReader* _in)
{
  m_levelDifficulty = -1;
  while (_in->ReadLine())
  {
    if (!_in->TokenAvailable())
      continue;
    char* word = _in->GetNextToken();

    if (stricmp("Difficulty_EndDefinition", word) == 0)
      return;
    if (stricmp(word, "CreatedAsDifficulty") == 0)
    {
      // The difficulty setting is 1-based in the file, but 0-based internally
      m_levelDifficulty = atoi(_in->GetNextToken()) - 1;
      if (m_levelDifficulty < 0)
        m_levelDifficulty = 0;
    }
  }
}

bool LevelFile::HasEffect(char* _effect)
{
  return false;
}

int LevelFile::CalculeCRC(const char* _mapFilename)
{
  int mapCRC = -1;

  if (!_mapFilename)
    return mapCRC;

  char filename[512];
  _snprintf(filename, sizeof(filename), "levels/%s", _mapFilename);
  filename[sizeof(filename) - 1] = '\0';

  // Determine CRC identifier of map
  BinaryReader* reader = g_app->m_resource->GetBinaryReader(filename);
  if (reader && reader->IsOpen())
    mapCRC = FileCRC(reader);
  delete reader;

  return mapCRC;
}
