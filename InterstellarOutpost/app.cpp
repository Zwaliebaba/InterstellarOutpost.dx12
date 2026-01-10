#include "pch.h"
#include "unicode_text_stream_reader.h"
#include "clienttoserver.h"
#include "metaserver_defines.h"
#include "metaserver.h"
#include "authentication.h"
#include "matchmaker.h"
#include "hi_res_time.h"
#include "language_table.h"
#include "mouse_cursor.h"
#include "preferences.h"
#include "resource.h"
#include "profiler.h"
#include "system_info.h"
#include "text_renderer.h"
#include "filesys_utils.h"
#include "bitmap.h"
#include "text_file_writer.h"
#include "prefs_other_window.h"
#include "window_manager.h"
#include "sound_stream_decoder.h"
#include "soundsystem.h"
#include "sample_cache.h"
#include "net_lib.h"
#include "app.h"
#include "camera.h"
#include "global_world.h"
#include "location.h"
#include "location_input.h"
#include "main.h"
#include "particle_system.h"
#include "renderer.h"
#include "script.h"
#include "user_input.h"
#include "taskmanager_interface.h"
#include "gamecursor.h"
#include "level_file.h"
#include "game_menu.h"
#include "multiwinia.h"
#include "shaman_interface.h"
#include "explosion.h"
#include "markersystem.h"
#ifdef BENCHMARK_AND_FTP
#include "benchmark.h"
#endif
#include "achievement_tracker.h"
#include "loading_screen.h"
#include "team.h"
#include "mapdata.h"
#include "GameMenuWindow.h"

void SetPreferenceOverrides(); // See main.cpp

App* g_app = NULL;

static bool s_profileDirectory = true;

App::App()
  : m_userInput(NULL),
    m_resource(NULL),
    m_soundSystem(NULL),
    m_particleSystem(NULL),
    m_langTable(NULL),
    m_globalWorld(NULL),
    m_location(NULL),
    m_locationId(-1),
    m_camera(NULL),
    m_server(NULL),
    m_clientToServer(NULL),
    m_originVersion("unknown"),
    m_mainThreadId(NetGetCurrentThreadId()),
    m_renderer(NULL),
    m_locationInput(NULL),
    m_effectProcessor(NULL),
    m_taskManagerInterface(NULL),
    m_script(NULL),
    m_testHarness(NULL),
    m_startSequence(NULL),
    m_gameMenu(NULL),
    m_multiwinia(NULL),
    m_shamanInterface(NULL),
    m_negativeRenderer(false),
    m_difficultyLevel(0),
    m_largeMenus(false),
    m_usingFontCopies(false),
    m_steamInited(false),
    m_userRequestsPause(false),
    m_lostFocusPause(false),
    m_editing(false),
    m_requestedLocationId(-1),
    m_requestToggleEditing(false),
    m_requestQuit(false),
    m_levelReset(false),
    m_atMainMenu(true),
    m_atLobby(false),
    m_gameMode(GameModeNone),
    m_loadingLocation(false),
    m_spectator(false),
    m_hideInterface(false),
    m_soundsLoaded(false),
    m_requireSoundsLoaded(false),
    m_doMenuTransition(false),
    m_checkedForPDLC(false),
    m_soundsWorkQueue(new WorkQueue),
    m_oldLangTable(NULL)
{
  g_app = this;

  m_netLib = new NetLib();
  m_netLib->Initialise();

  m_resource = new Resource();

  g_prefsManager = new PrefsManager(GetPreferencesPath());
  SetPreferenceOverrides();

  m_renderer = new Renderer();
  m_renderer->Initialise();
  m_renderer->SetOpenGLState();

#ifdef SPECTATOR_ONLY
  m_spectator = true;
#endif

  g_loadingScreen->m_workQueue->Add(&App::Initialise, this);

  // Need to serialise the loading of the sounds on PC 
  // Can't unrar too files at once (unrar not thread safe, regrettably)
  g_loadingScreen->Render();
  m_soundsWorkQueue->Add(&App::LoadSounds, this);
}

App::~App()
{
#ifdef BENCHMARK_AND_FTP
  SAFE_DELETE(m_benchMark);
#endif
  SAFE_DELETE(m_gameMenu);
  SAFE_DELETE(m_multiwinia);
  SAFE_DELETE(m_globalWorld);
  SAFE_DELETE(m_langTable);
  SAFE_DELETE(m_taskManagerInterface);
  SAFE_DELETE(m_shamanInterface);
  SAFE_DELETE(m_script);
  SAFE_DELETE(m_particleSystem);
  SAFE_DELETE(m_markerSystem);
  SAFE_DELETE(m_camera);
  SAFE_DELETE(m_userInput);
  SAFE_DELETE(m_clientToServer);
  SAFE_DELETE(m_soundSystem);
  SAFE_DELETE(m_gameCursor);
  SAFE_DELETE(m_renderer);
  SAFE_DELETE(g_prefsManager);
  SAFE_DELETE(m_soundsWorkQueue);
  SAFE_DELETE(m_resource);
  SAFE_DELETE(m_netLib);
  SAFE_DELETE(m_achievementTracker);
}

void App::SetProfileName(const char* _profileName)
{
  strcpy(m_userProfileName, _profileName);

  DebugTrace("Setting ProfileName to %s\n", _profileName);

  if (stricmp(_profileName, "AttractMode") != 0)
  {
    g_prefsManager->SetString("UserProfile", m_userProfileName);
    g_prefsManager->Save();
  }
}

bool App::LoadProfile()
{
  bool newProfile = m_globalWorld->m_loadingNewProfile;
  if (stricmp(m_userProfileName, "AccessAllAreas") == 0)
  {
    // Cheat username that opens all locations
    // aimed at beta testers who've completed the game already

    if (m_globalWorld)
    {
      delete m_globalWorld;
      m_globalWorld = NULL;
    }

    m_globalWorld = new GlobalWorld();
    m_globalWorld->m_loadingNewProfile = newProfile;
    m_globalWorld->LoadGame("game_unlockall.txt");
    for (int i = 0; i < m_globalWorld->m_buildings.Size(); ++i)
    {
      GlobalBuilding* building = m_globalWorld->m_buildings[i];
      if (building && building->m_type == Building::TypeTrunkPort)
        building->m_online = true;
    }
    for (int i = 0; i < m_globalWorld->m_locations.Size(); ++i)
    {
      GlobalLocation* loc = m_globalWorld->m_locations[i];
      loc->m_available = true;
    }
  }
  else
  {
    if (m_globalWorld)
    {
      delete m_globalWorld;
      m_globalWorld = NULL;
    }

    m_globalWorld = new GlobalWorld();
    m_globalWorld->m_loadingNewProfile = newProfile;
    DebugTrace("We are %sloading a new profile\n", newProfile ? "" : "not ");
    m_globalWorld->LoadGame(m_gameDataFile);
  }

  return true;
}

bool App::SaveProfile(bool _global, bool _local)
{
  bool canWrite = true;

  char folderName[512];
  sprintf(folderName, "%susers/", GetProfileDirectory());
  bool success = CreateDirectory(folderName);
  if (!success)
  {
    DebugTrace("failed to create folder %s\n", folderName);
    return false;
  }

  sprintf(folderName, "%susers/%s", GetProfileDirectory(), m_userProfileName);
  success = CreateDirectory(folderName);
  if (!success)
  {
    DebugTrace("failed to create folder %s\n", folderName);
    return false;
  }

#ifdef TARGET_OS_VISTA
  if (_global) { SaveRichHeader(); }
#endif

  if (canWrite && _global)
    m_globalWorld->SaveGame(m_gameDataFile);

  bool returnVal = true;

  if (canWrite && _local && g_app->m_location)
  {
    if (m_levelReset)
    {
      m_levelReset = false;
      returnVal = false;
    }
    else
    {
      g_app->m_location->m_levelFile->GenerateInstantUnits();
      g_app->m_location->m_levelFile->GenerateDynamicBuildings();
      char* missionFilename = m_location->m_levelFile->m_missionFilename;
      m_location->m_levelFile->SaveMissionFile(missionFilename);
    }
  }

  return returnVal;
}

void App::ResetLevel(bool _global)
{
  if (m_location)
  {
    m_requestedLocationId = -1;
    m_requestedMission[0] = '\0';
    m_requestedMap[0] = '\0';

    //
    // Delete the saved mission file

    char* missionFilename = m_location->m_levelFile->m_missionFilename;
    char saveFilename[256];
    sprintf(saveFilename, "%susers/%s/%s", GetProfileDirectory(), m_userProfileName, missionFilename);

    DeleteThisFile(saveFilename);

    m_levelReset = true;

    //
    // Delete the game file if required

    if (_global)
    {
      sprintf(saveFilename, "%susers/%s/%s", GetProfileDirectory(), m_userProfileName, m_gameDataFile);

      DeleteThisFile(saveFilename);

      if (m_globalWorld)
      {
        delete m_globalWorld;
        m_globalWorld = NULL;
      }

      m_globalWorld = new GlobalWorld();
      m_globalWorld->LoadGame(m_gameDataFile);
    }
  }
}

void App::HandleDelayedJobs()
{
  if (m_delayedJobListMutex.TryLock())
  {
    DelayedJob* dJob = m_delayedJobs.GetData(0);

    if (dJob && dJob->ReadyToRun())
    {
      dJob->Run();
      m_delayedJobs.RemoveData(0);
      delete dJob;
    }
    m_delayedJobListMutex.Unlock();
  }
}

void App::AddDelayedJob(DelayedJob* _dJob)
{
  m_delayedJobListMutex.Lock();
  m_delayedJobs.PutDataAtEnd(_dJob);
  m_delayedJobListMutex.Unlock();
}

void App::StartNetwork(bool _iAmAServer, const char* _serverIp, int _serverPort)
{
  if (!g_app->m_editing)
  {
    char serverIp[16];

    if (_iAmAServer)
    {
      delete m_server;
      m_server = new Server();
      m_server->Initialise();
      m_server->m_noAdvertise = true;

      if (_serverPort != -1)
      {
        GetLocalHostIP(serverIp, sizeof(serverIp));
        _serverIp = serverIp;
        _serverPort = g_app->m_server->m_listener->GetPort();
      }
      else
        _serverIp = "127.0.0.1";
    }

    m_clientToServer->ClientJoin(_serverIp, _serverPort);
  }
}

bool App::StartSinglePlayerServer()
{
  DebugTrace("Starting single player server.\n");
  m_multiwinia->m_aiType = Multiwinia::AITypeStandard;
  g_gameTimer.Reset();
  NetLockMutex lock(m_networkMutex);
  g_app->StartNetwork(true, NULL, -1);

  return true;
}

HRESULT App::StartMultiPlayerServer()
{
  DebugTrace("Starting multi-player server.\n");
  m_multiwinia->m_aiType = Multiwinia::AITypeStandard;
  g_app->StartNetwork(true, NULL, NULL);
  return 0; // = S_OK = success
}

void App::ShutdownCurrentGame()
{
  SaveProfile(false, true);

  g_explosionManager.Reset();

  if (m_location)
    m_globalWorld->TransferSpirits(m_locationId);

  m_clientToServer->ClientLeave();

  if (m_location)
    m_location->Empty();

  m_particleSystem->Empty();
  m_markerSystem->ClearAllMarkers();

  delete m_location;

  m_location = NULL;
  m_locationId = -1;

  delete m_locationInput;
  m_locationInput = NULL;

  delete m_server;
  m_server = NULL;

  m_multiwinia->Reset();

  m_globalWorld->m_myTeamId = 255;
  m_globalWorld->EvaluateEvents();

  m_userRequestsPause = false;

  SaveProfile(true, false);
}

std::string App::GetFirstAvailableLanguage()
{
  auto files = g_app->m_resource->ListResources("%s\\language\\", "*.*", FileSys::GetHomeDirectoryA().c_str());
  if (files.size() > 0)
  {
    std::string first = files[0];
    return first.substr(0, first.find('.'));
  }

  return "unknown";
}

const char* App::GetDefaultLanguage() { return g_systemInfo->m_localeInfo.m_language; }

void App::InitLanguage()
{
  std::list<std::string> languagePreference;

  languagePreference.push_back(GetDefaultLanguage());
  languagePreference.push_back("english");
  languagePreference.push_back(GetFirstAvailableLanguage());

  ASSERT_TEXT(languagePreference.end() != std::find_if(languagePreference.begin(), languagePreference.end(),
                                                       [this](const std::string& lang) { return this->TrySetLanguage(lang); }),
              "Failed to load language file");
}

void App::SetLanguage(const char* _language, bool _test)
{
  DebugTrace("Setting language to {} (Test = {})\n", _language, _test);

  //
  // Load the language text file

  char langFilename[256];
#if defined(TARGET_OS_LINUX) && defined(TARGET_DEMOGAME)
  sprintf(langFilename, "language/%s_demo.txt", _language);
#else
  sprintf(langFilename, "language\\%s.txt", _language);
#endif

  auto newLangTable = new LangTable(langFilename);

  if (_test)
    newLangTable->TestAgainstEnglish();

  // Set the locale so that Uppercasing works correctly
  setlocale(LC_CTYPE, _language);

  //
  // Load the MOD language file if it exists

  sprintf(langFilename, "strings_%s.txt", _language);
  TextReader* modLangFile = g_app->m_resource->GetTextReader(langFilename);
  if (!modLangFile)
  {
    sprintf(langFilename, "strings_default.txt");
    modLangFile = g_app->m_resource->GetTextReader(langFilename);
  }

  if (modLangFile)
  {
    delete modLangFile;
    newLangTable->ParseLanguageFile(langFilename);
  }

  DebugTrace("Loading fonts\n");

  static bool initedFonts = false;
  if (!initedFonts)
  {
    g_gameFont.Initialise("speccy");
    g_editorFont.Initialise("editor");
    g_titleFont.Initialise("square");

    g_editorFont.SetHorizSpacingFactor(0.91f);
    g_titleFont.SetHorizSpacingFactor(1.03f);

    initedFonts = true;
  }

  if (strcmp(_language, "chinese-trad") == 0 || strcmp(_language, "chinese-simp") == 0 || strcmp(_language, "korean") == 0 ||
    strcmp(_language, "japanese") == 0)
  {
    g_gameFont.SetHorizScaleFactor(1.0f);
    g_editorFont.SetHorizScaleFactor(1.0f);
    g_titleFont.SetHorizScaleFactor(1.0f);

    g_oldEditorFont = g_editorFont;
    g_oldGameFont = g_gameFont;

    g_editorFont = g_titleFont;
    g_gameFont = g_titleFont;
    m_usingFontCopies = true;
  }
  else
  {
    if (m_usingFontCopies)
      g_editorFont = g_oldEditorFont;
    if (m_usingFontCopies)
      g_gameFont = g_oldGameFont;

    g_editorFont.SetHorizScaleFactor(0.6f);
    g_gameFont.SetHorizScaleFactor(0.6f);
    g_titleFont.SetHorizScaleFactor(0.6f);

    m_usingFontCopies = false;
  }

  DebugTrace("Fonts loaded.\n");

  if (g_inputManager)
    newLangTable->RebuildTables();

  // Delete the old table, if it exists (which it doesn't when Multiwinia is first run)
  if (m_langTable != NULL)
  {
    m_oldLangTable = m_langTable;
    Method<App>* m = new Method<App>(&App::DeleteOldLangTable, this);
    DelayedJob* dJob = new DelayedJob(m, 5);
    AddDelayedJob(dJob);
  }

  m_langTable = newLangTable;
}

bool App::TrySetLanguage(std::string _language)
{
  std::string languageFile = "language\\" + _language + ".txt";

  if (!m_resource->FileExists(languageFile.c_str()))
    return false;

  const char* language = _language.c_str();
  g_prefsManager->SetString("TextLanguage", language);

  SetLanguage(language, g_prefsManager->GetInt("TextLanguageTest", 0));
  return true;
}

void App::InitMetaServer()
{
  char key[256], path[512];

  strcpy(path, GetProfileDirectory());
  char fullFileName[512];
  sprintf(fullFileName, "%sauthkey.dev", path);

  Authentication_LoadKey(key, fullFileName);
  Authentication_SetKey(key);

  Authentication_RequestStatus(key, METASERVER_GAMETYPE_MULTIWINIA);
  auto metaServerLocation = "metaserver-mwdev.introversion.co.uk";

  MetaServer_Initialise();
  MetaServer_Connect(metaServerLocation, PORT_METASERVER_CLIENT_LISTEN);
  MatchMaker_LocateService(metaServerLocation, PORT_METASERVER_LISTEN);
}

bool App::Multiplayer() { return g_app->m_gameMode == GameModeMultiwinia; }

bool App::IsSinglePlayer() { return (g_app->m_gameMode == GameModeCampaign || g_app->m_gameMode == GameModePrologue); }

void App::UseProfileDirectory(bool _profileDirectory) { s_profileDirectory = _profileDirectory; }

const char* App::GetProfileDirectory() { return ""; }

const char* App::GetPreferencesPath()
{
  static char* path = NULL;
  if (path == NULL)
  {
#if defined(TARGET_OS_MACOSX)
    const char* home = getenv("HOME"); if (home != NULL)
    {
      path = new char[strlen(home) + 256];
      sprintf(path, "%s/Library/Preferences/uk.co.introversion.%s.txt", home, APP_NAME);
    }
#else
    const char* profileDir = GetProfileDirectory();
    path = new char[strlen(profileDir) + 32];
    sprintf(path, "%spreferences.txt", profileDir);
#endif
  }

  return path;
}

const char* App::GetScreenshotDirectory()
{
#if defined(TARGET_OS_VISTA)
  static char dir[MAX_PATH]; SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, dir); sprintf(dir, "%s\\", dir); return dir;
#elif defined(TARGET_OS_MACOSX)
  FSRef ref; static char dir[1024] = ""; if (strlen(dir) == 0)
  {
    if (FSFindFolder(kUserDomain, kPictureDocumentsFolderType, kCreateFolder, &ref) != noErr)
      return NULL;
    if (FSRefMakePath(&ref, (UInt8*)dir, sizeof(dir) - 32) != noErr)
      return NULL;
    strcat(dir, "/Multiwinia/");
    // note that it's okay for the directory to not exist yet
  } return dir;
#else
  static char* path = NULL;

  if (path == NULL)
  {
    const char* profileDir = GetProfileDirectory();
    path = new char[strlen(profileDir) + 32];
    sprintf(path, "%sscreenshots/", profileDir);
  }

  return path;
#endif
}

const char* App::GetMapDirectory()
{
  static char* directory = NULL;
  if (!directory)
  {
    const char* profileDir = GetProfileDirectory();
    directory = new char[strlen(profileDir) + 32];
    sprintf(directory, "%smaps/", profileDir);
  }

  return directory;
}

void App::UpdateDifficultyFromPreferences()
{
  // This method is called to make sure that the difficulty setting
  // used to control the game play (g_app->m_difficultyLevel) is 
  // consistent with the user preferences. 

  // Preferences value is 1-based, m_difficultyLevel is 0-based.
  m_difficultyLevel = 0;
  if (m_difficultyLevel < 0)
    m_difficultyLevel = 0;
}

bool App::ToggleGamePaused()
{
  bool allowPause = true;
  for (int i = 0; i < NUM_TEAMS; ++i)
  {
    if (m_multiwinia->m_teams[i].m_teamType == TeamTypeRemotePlayer)
    {
      allowPause = false;
      break;
    }
  }
  if (m_multiwinia->GameInGracePeriod() || m_multiwinia->GameOver())
    allowPause = false;
  if (allowPause)
    m_clientToServer->RequestPause();

  return allowPause;
}

bool App::GamePaused() const
{
  return m_location && (g_gameTimer.IsPaused() || m_clientToServer->m_outOfSyncClients.Size() > 0 || (m_lostFocusPause && g_app->
    IsSinglePlayer()));
}

bool App::EarnedAchievement(int _achievementId) { return false; }

void App::CheckMasterAchievement() {}

void App::GiveAchievement(int _achievementId)
{
  // achievements should only be available in games without AI players
  if (_achievementId != DarwiniaAchievementMasterPlayer && _achievementId != DarwiniaAchievementCarnage && _achievementId !=
    DarwiniaAchievementGenocide)
  {
    for (int i = 0; i < g_app->m_location->m_levelFile->m_numPlayers; ++i)
    {
      if (m_multiwinia->m_teams[i].m_teamType == TeamTypeCPU)
        return;
    }
  }

  if (EarnedAchievement(_achievementId))
    return;
}

int App::GetMapID(int gameMode, int mapCrcId)
{
  if (gameMode < 0 || gameMode >= MAX_GAME_TYPES)
    return -1;

  DArray<MapData*>& maps = g_app->m_gameMenu->m_maps[gameMode];

  for (int i = 0; i < maps.Size(); i++)
  {
    if (maps.ValidIndex(i) && maps[i]->m_mapId == mapCrcId)
      return i;
  }

  return -1;
}

const char* App::GetBuyNowURL() const { return "http://store.introversion.co.uk"; }

void App::CheckSounds()
{
  if (!m_soundSystem->IsInitialized())
    m_soundSystem->Initialise();

  if (m_soundSystem->IsInitialized())
    g_cachedSampleManager.CleanUp();
}

int App::GetMaxNumberofPlayers()
{

  int gameType = m_multiwinia->m_gameType;

  MapData* mapData = NULL;

  // Determine how many players can join this game
  if (gameType != -1 && m_requestedMap)
  {
    DArray<MapData*>& maps = m_gameMenu->m_maps[gameType];

    for (int i = 0; i < maps.Size(); i++)
    {
      if (maps.ValidIndex(i))
      {
        MapData* m = maps[i];
        if (stricmp(g_app->m_requestedMap, m->m_fileName) == 0)
        {
          mapData = m;
          return m->m_numPlayers;
        }
      }
    }
  }

  return 0;
}

bool App::UseChristmasMode()
{
  if (g_app->m_editing)
    return false;

  // Last 2 weeks in December only
  // Also allow user to disable if he wishes

  time_t now = time(NULL);
  tm* theTime = localtime(&now);

#ifdef CHRISTMAS_DEMO
  if (theTime->tm_mon == 10 && theTime->tm_mday >= 27)
    return true; if (theTime->tm_mon == 11)
    return true;
#else
  if (theTime->tm_mon == 11)
  {
    if (theTime->tm_mday == 24 || theTime->tm_mday == 25 || theTime->tm_mday == 26)
      return true;
  }
#endif

  return false;
}

void App::Initialise()
{
  strcpy(m_requestedMission, "null");
  strcpy(m_requestedMap, "null");

  // Load resources

  double start = GetHighResTime();

  InitLanguage();

  //m_soundsLoaded = true;

  m_negativeRenderer = g_prefsManager->GetInt("RenderNegative", 0) ? true : false;
  if (m_negativeRenderer)
    m_backgroundColour.Set(255, 255, 255, 255);
  else
    m_backgroundColour.Set(0, 0, 0, 0);

  UpdateDifficultyFromPreferences();

  m_gameCursor = new GameCursor();
  m_markerSystem = new MarkerSystem();
  m_soundSystem = new SoundSystem();
  m_clientToServer = new ClientToServer();

  DebugTrace("Inits 1: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  InitMetaServer();
  DebugTrace("Inits 2: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  m_clientToServer->OpenConnections();

  m_userInput = new UserInput();
  m_camera = new Camera();

  strcpy(m_gameDataFile, "game.txt");

  DebugTrace("Inits 3: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  SetProfileName(g_prefsManager->GetString("UserProfile", "none"));

  m_particleSystem = new ParticleSystem();

  m_script = new Script();
  m_shamanInterface = new ShamanInterface();

  DebugTrace("Inits 4: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  int menuOption = g_prefsManager->GetInt(OTHER_LARGEMENUS, 0);
  if (menuOption == 2) // (todo) or is running in media center and tenFootMode == -1
    m_largeMenus = true;

  DebugTrace("Inits 5: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  m_achievementTracker = new AchievementTracker();
  m_multiwinia = new Multiwinia();
  m_gameMenu = new GameMenu();

  DebugTrace("Inits 6: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

#ifdef BENCHMARK_AND_FTP
  m_benchMark = new BenchMark(); m_benchMark->RequestDXDiag();
#endif

  DebugTrace("Inits 7: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  DebugTrace("Inits 8: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  //
  // Load save games

  // bool profileLoaded = LoadProfile();
  m_globalWorld = new GlobalWorld;

  DebugTrace("Inits 10: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  TaskManagerInterface::CreateTaskManager();

  DebugTrace("Inits 11: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  GameMenuWindow::PreloadTextures();

  DebugTrace("Inits 11b: %f\n", GetHighResTime() - start);
  start = GetHighResTime();
}

void App::LoadSounds()
{
  m_soundsLoaded = true;

  if (m_soundSystem)
    m_soundSystem->TriggerInitialize();
}

void App::DeleteOldLangTable()
{
  if (m_oldLangTable != NULL)
  {
    delete m_oldLangTable;
    m_oldLangTable = NULL;
  }
}
