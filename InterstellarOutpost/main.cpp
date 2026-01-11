#include "pch.h"
#include "eclipse.h"
#include "unicode_text_stream_reader.h"
#include "hi_res_time.h"
#include "input.h"
#include "file_paths.h"
#include "targetcursor.h"
#include "math_utils.h"
#include "preferences.h"
#include "profiler.h"
#include "system_info.h"
#include "text_renderer.h"
#include "vector3.h"
#include "window_manager.h"
#include "resource.h"
#include "text_stream_readers.h"
#include "language_table.h"
#include "random_number.h"
#include "filesys_utils.h"
#include "win32_eventhandler.h"
#include "inputdriver_win32.h"
#include "inputdriver_prefs.h"
#include "inputdriver_alias.h"
#include "inputdriver_conjoin.h"
#include "inputdriver_chord.h"
#include "inputdriver_invert.h"
#include "inputdriver_idle.h"
#include "inputdriver_value.h"
#include "prefs_other_window.h"
#include "sound_library_2d.h"
#include "sound_library_3d_software.h"
#include "sample_cache.h"
#include "app.h"
#include "camera.h"
#include "explosion.h"
#include "global_world.h"
#include "landscape.h"
#include "location.h"
#include "location_input.h"
#include "main.h"
#include "renderer.h"
#include "script.h"
#include "soundsystem.h"
#include "sample_cache.h"
#include "taskmanager_interface.h"
#include "team.h"
#include "user_input.h"
#include "game_menu.h"
#include "shaman_interface.h"
#include "markersystem.h"
#ifdef BENCHMARK_AND_FTP
#include "benchmark.h"
#endif
#include "loading_screen.h"
#include "level_file.h"
#include "entity_grid.h"
#include "mainmenus.h"
#include "debugmenu.h"
#include "clienttoserver.h"
#include "server.h"
#include "servertoclientletter.h"
#include "network_defines.h"
#include "iframe.h"
#include "ftp_manager.h"
#include "gunturret.h"
#include "darwinian.h"
#include "achievement_tracker.h"

#define TARGET_FRAME_RATE_INCREMENT 0.25

static void Finalise();

// ******************
//  Global Variables
// ******************

double g_gameTime = 0.0;
double g_advanceTime;
int g_lastProcessedSequenceId = -1;
int g_sliceNum; // Most recently advanced slice
bool g_RenderingFirstTimeBL = false;

bool requestBootloader = false;

// ******************
//  Local Functions
// ******************

void UpdateAdvanceTime()
{
    if (!g_app->GamePaused())
    {
      double realTime = GetHighResTime();
      g_advanceTime = realTime - g_gameTime;
      if (g_advanceTime > 0.25)
        g_advanceTime = 0.25;
      g_gameTime = realTime;
    }
}

bool WindowsOnScreen() { return EclGetWindows()->Size() > 0; }

void RemoveAllWindows()
{
  if (g_app->m_location)
  {
    int x = g_app->m_renderer->ScreenW() / 2;
    int y = g_app->m_renderer->ScreenH() / 2;
    g_target->SetMousePos(x, y);
  }

  LList<EclWindow*>* windows = EclGetWindows();
  while (windows->Size() > 0)
  {
    EclWindow* w = windows->GetData(0);
    EclRemoveWindow(w->m_name);
  }
}

bool HandleCommonConditions()
{
  g_app->HandleDelayedJobs();

  bool curWindowHasFocus = g_eventHandler->WindowHasFocus();

  g_app->m_lostFocusPause = !curWindowHasFocus;

  if (g_inputManager->controlEvent(ControlGameDebugQuit))
    g_app->m_requestQuit = true;

  g_app->CheckSounds();

  if (g_app->m_requestQuit)
  {
    Finalise();
    exit(0);
  }

  return false;
}

void SendTeamControlsToNetwork(TeamControls& teamControls)
{
  // Read the current teamControls from the inputManager				

  bool chatLog = g_app->m_camera->ChatLogVisible();

  Team* team = g_app->m_location->GetMyTeam();
  if (team)
  {
    // We don't actually want to pass any left-clicks to the network system
    // If the user has left-clicked on another of his entities, because that 
    // entity is about to be selected.  We don't want our original entity 
    // walking up to him.
    WorldObjectId idUnderMouse;
    bool objectUnderMouse = g_app->m_locationInput->GetObjectUnderMouse(idUnderMouse, g_app->m_globalWorld->m_myTeamId);

    if (objectUnderMouse)
    {
      Entity* entity = g_app->m_location->GetEntity(idUnderMouse);
      if (entity)
        teamControls.m_mousePos = entity->m_pos;
    }

    if (idUnderMouse.GetUnitId() == UNIT_BUILDINGS)
    {
      // Focus the mouse on a Radar Dish if one exists under the mouse
      Building* building = g_app->m_location->GetBuilding(idUnderMouse.GetUniqueId());
      if (building && building->m_type == Building::TypeRadarDish)
        teamControls.m_mousePos = building->m_pos;
    }
  }

  if (g_app->m_taskManagerInterface->IsVisible() || EclGetWindows()->Size() != 0 || chatLog /*entityUnderMouse || */)
    teamControls.ClearFlags();

  g_app->m_clientToServer->SendIAmAlive(g_app->m_globalWorld->m_myTeamId, teamControls);

  teamControls.Clear();
}

void LocationGameLoop()
{
  g_app->m_loadingLocation = false;
  g_app->m_hideInterface = false;

  bool iAmAClient = true;
  bool iAmAServer = (g_app->m_server != NULL);

  double nextIAmAliveMessage = GetHighResTime();
  double lastRenderTime = GetHighResTime();

  TeamControls teamControls;

  g_app->m_renderer->StartFadeIn(0.6f);
  g_app->m_soundSystem->TriggerOtherEvent(NULL, "EnterLocation", SoundSourceBlueprint::TypeAmbience);

  //
  // Main loop

  bool multiwiniaCam = false;
  bool fadingOut = false;

  while (true)
  {
    if (!fadingOut)
    {
      if (g_app->m_requestedLocationId != g_app->m_locationId)
      {
        g_app->m_renderer->StartFadeOut();
        fadingOut = true;
      }
    }
    else
    {
      if (g_app->m_renderer->IsFadeComplete())
      {
        break;
      }
    }

    if (!multiwiniaCam && g_app->m_gameMode == App::GameModeMultiwinia)
    {
      // this has to be done here because you cant guarentee the existance of the team before entering the loop
      if (g_app->m_location->GetMyTeam())
      {
        char camname[64];
        sprintf(camname, "player%d", g_app->m_location->GetTeamPosition(g_app->m_globalWorld->m_myTeamId));
        g_app->m_camera->SetTarget(camname);
        g_app->m_camera->CutToTarget();
        multiwiniaCam = true;
      }
    }

    g_inputManager->PollForEvents();
    if (g_inputManager->controlEvent(ControlToggleInterface))
      g_app->m_hideInterface = !g_app->m_hideInterface;
    if (g_inputManager->controlEvent(ControlMenuEscape) && g_app->m_renderer->IsFadeComplete())
    {
      if (g_app->m_script && g_app->m_script->IsRunningScript())
      {
#ifdef USE_SEPULVEDA_HELP_TUTORIAL
        g_app->m_sepulveda->PlayerSkipsMessage();
#endif
      }
      else
      {
        if (WindowsOnScreen())
        {
          RemoveAllWindows();
          g_app->m_hideInterface = false;
        }
        else if (g_app->m_taskManagerInterface->IsVisible())
          g_app->m_taskManagerInterface->SetVisible(false);
        else
        {
          g_app->m_camera->SetDebugMode(Camera::DebugModeAuto);
          EclRegisterWindow(new LocationWindow());
          g_app->m_hideInterface = true;
        }
      }
      g_app->m_userInput->Advance();
    }

    // Clear out any display lists wanting to be created
    g_loadingScreen->RunJobs();

    if (HandleCommonConditions())
      continue;

    //
    // Get the time
    double timeNow = GetHighResTime();

    //
    // Advance the server
    if (iAmAServer)
      g_app->m_server->AdvanceIfNecessary(timeNow);

    if (!WindowsOnScreen())
      teamControls.Advance();

    if (iAmAClient)
    {
      START_PROFILE("Client Main Loop");

      //
      // Send Client input to Server
      if (timeNow > nextIAmAliveMessage)
      {
        SendTeamControlsToNetwork(teamControls);

        nextIAmAliveMessage += IAMALIVE_PERIOD;
        if (timeNow > nextIAmAliveMessage)
          nextIAmAliveMessage = timeNow + IAMALIVE_PERIOD;
      }

      g_app->m_clientToServer->AdvanceAndCatchUp();

      if (g_app->m_multiwinia->m_resetLevel)
      {
        g_app->m_multiwinia->ResetLocation();
        multiwiniaCam = false;
      }

      END_PROFILE("Client Main Loop");

      // Render
      UpdateAdvanceTime();
      lastRenderTime = GetHighResTime();
      if (g_profiler)
        g_profiler->Advance();

      g_app->m_userInput->Advance();

      // The following are candidates for running in parallel
      // using something like OpenMP
      bool soundSystemIsInitialized = g_app->m_soundSystem->IsInitialized();
      if (soundSystemIsInitialized)
        g_soundLibrary2d->TopupBuffer();

      if (g_eventHandler->WindowHasFocus())
        g_app->m_camera->Advance();

      g_app->m_locationInput->Advance();
      g_app->m_taskManagerInterface->Advance();
      g_app->m_script->Advance();
      g_explosionManager.Advance();

      if (soundSystemIsInitialized)
        g_app->m_soundSystem->Advance();
      g_app->m_shamanInterface->Advance();
      g_app->m_markerSystem->Advance();
#ifdef BENCHMARK_AND_FTP
      g_app->m_benchMark->Advance();
#endif

      // DELETEME: for debug purposes only
      g_app->m_globalWorld->EvaluateEvents();

      g_app->m_renderer->Render();

      if (g_app->m_renderer->m_fps < 15)
      {
        if (soundSystemIsInitialized)
          g_app->m_soundSystem->Advance();
      }
    }
  }

  g_app->m_soundSystem->StopAllSounds(WorldObjectId(), "Ambience EnterLocation");
  g_app->m_soundSystem->TriggerOtherEvent(NULL, "ExitLocation", SoundSourceBlueprint::TypeAmbience);
  g_app->ShutdownCurrentGame();
}

#ifdef LOCATION_EDITOR
#ifndef PURITY_CONTROL
void LocationEditorLoop()
{
  while (!g_inputManager->controlEvent(ControlMenuEscape))
  {
    g_inputManager->PollForEvents();

    if (HandleCommonConditions())
      continue;

    //
    // Get the time
    UpdateAdvanceTime();
    double timeNow = GetHighResTime();

    g_app->m_userInput->Advance();
    g_app->m_camera->Advance();
    g_app->m_locationEditor->Advance();
    if (g_app->m_soundSystem->IsInitialized())
      g_app->m_soundSystem->Advance();
    if (g_profiler)
      g_profiler->Advance();

    g_app->m_renderer->Render();
  }

  RemoveAllWindows();

  if (strcmp(GetExtensionPart(g_app->m_location->m_levelFile->m_mapFilename), "mwm") == 0)
  {
    char filename[512];
    sprintf(filename, "%s%s", g_app->GetMapDirectory(), g_app->m_location->m_levelFile->m_mapFilename);
    MapFile* mapFile = new MapFile(filename, true);
    TextReader* in = mapFile->GetTextReader();
    g_app->m_gameMenu->RemoveMap(g_app->m_location->m_levelFile->m_mapFilename);
    g_app->m_gameMenu->AddMap(in, g_app->m_location->m_levelFile->m_mapFilename);
    delete in;
    delete mapFile;
  }

  delete g_app->m_locationEditor;
  g_app->m_locationEditor = NULL;

  g_app->m_location->Empty();
  delete g_app->m_location;
  g_app->m_location = NULL;
  g_app->m_locationId = -1;
  g_app->m_requestedLocationId = -1;

  delete g_app->m_locationInput;
  g_app->m_locationInput = NULL;

  g_app->m_editing = false;
  g_app->m_atMainMenu = true;

  if (g_prefsManager->GetInt("RecordDemo") == 1)
  {
    if (g_app->m_server)
      g_app->m_server->SaveHistory("serverHistory.dat");
    //g_inputManager->StopLogging( "inputLog.dat" );
  }
}
#endif // PURITY_CONTROL
#endif // LOCATION_EDITOR

void GlobalWorldGameLoop()
{
  g_app->m_renderer->StartFadeIn(0.25f);

  g_app->m_soundSystem->TriggerOtherEvent(NULL, "EnterGlobalWorld", SoundSourceBlueprint::TypeAmbience);

  g_app->m_hideInterface = false;
  g_app->m_atMainMenu = true;
  while (g_app->m_requestedLocationId == -1)
  {
    if (g_app->m_atMainMenu)
      break;

    g_inputManager->PollForEvents();
    if (g_inputManager->controlEvent(ControlToggleInterface))
      g_app->m_hideInterface = !g_app->m_hideInterface;

    if (g_inputManager->controlEvent(ControlMenuEscape) && g_app->m_renderer->IsFadeComplete())
    {
      if (WindowsOnScreen())
        RemoveAllWindows();
      else
      {
        g_app->m_camera->SetDebugMode(Camera::DebugModeAuto);
        EclRegisterWindow(new MainMenuWindow());
      }
      g_app->m_userInput->Advance();
    }

    if (HandleCommonConditions())
      continue;

    // Get the time
    UpdateAdvanceTime();
    double timeNow = GetHighResTime();

    if (g_app->m_server)
      g_app->m_server->AdvanceIfNecessary(timeNow);
    g_app->m_clientToServer->AdvanceAndCatchUp();
    g_app->m_script->Advance();
#ifdef USE_SEPULVEDA_HELP_TUTORIAL
    g_app->m_sepulveda->Advance();
#endif
    g_app->m_globalWorld->Advance();
    g_app->m_userInput->Advance();
    g_app->m_camera->Advance();
    if (g_app->m_soundSystem->IsInitialized())
      g_app->m_soundSystem->Advance();

#ifdef ATTRACTMODE_ENABLED
    g_app->m_attractMode->Advance();
#endif
    if (g_profiler)
      g_profiler->Advance();

    g_app->m_globalWorld->EvaluateEvents();

    g_app->m_renderer->Render();
  }

  g_app->m_soundSystem->StopAllSounds(WorldObjectId(), "Ambience EnterGlobalWorld");
}

void SetPreferenceOverrides()
{
}

void InitialiseInputManager()
{
  g_inputManager = new InputManager;
  g_inputManager->addDriver(new ConjoinInputDriver());
  g_inputManager->addDriver(new ChordInputDriver());
  g_inputManager->addDriver(new InvertInputDriver());
  g_inputManager->addDriver(new IdleInputDriver());
  g_inputManager->addDriver(new W32InputDriver()); 
  g_inputManager->addDriver(new PrefsInputDriver());
  g_inputManager->addDriver(new ValueInputDriver());
  g_inputManager->addDriver(new AliasInputDriver());
}

void ReadInputPreferences()
{
  // Read Darwinia default input preferences file
  TextReader* inputPrefsReader = g_app->m_resource->GetTextReader(InputPrefs::GetSystemPrefsPath());
  if (inputPrefsReader)
  {
    ASSERT_TEXT(inputPrefsReader->IsOpen(), "Couldn't open input preferences file: %s\n", InputPrefs::GetSystemPrefsPath().c_str());
    g_inputManager->parseInputPrefs(*inputPrefsReader);
    delete inputPrefsReader;
  }

  // Override defaults with keyboard specific file, if applicable
  TextReader* localeInputPrefsReader = g_app->m_resource->GetTextReader(InputPrefs::GetLocalePrefsPath());
  if (localeInputPrefsReader)
  {
    if (localeInputPrefsReader->IsOpen())
      g_inputManager->parseInputPrefs(*localeInputPrefsReader, true);
    delete localeInputPrefsReader;
  }

  // Override again with user specified bindings
  TextReader* userInputPrefsReader = new UnicodeTextFileReader(InputPrefs::GetUserPrefsPath().c_str());

  if (userInputPrefsReader && !userInputPrefsReader->IsUnicode())
  {
    delete userInputPrefsReader;
    userInputPrefsReader = new TextFileReader(InputPrefs::GetUserPrefsPath().c_str());
  }

  if (userInputPrefsReader)
  {
    if (userInputPrefsReader->IsOpen())
      g_inputManager->parseInputPrefs(*userInputPrefsReader, true);
    delete userInputPrefsReader;
  }
}

void Initialise()
{
  DebugTrace("{} {} built {}\n", APP_NAME, APP_VERSION, __DATE__);

  //
  // Initialise all our basic objects

  g_systemInfo = new SystemInfo;
  InitialiseHighResTime();

  double startInit = GetHighResTime();

  g_eventHandler = new W32EventHandler();
  g_loadingScreen = new LoadingScreen();

  InitialiseInputManager();
  g_app = new App();

  double start = GetHighResTime();
  ReadInputPreferences();
  DebugTrace("Inits 12: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

#if defined(TARGET_OS_VISTA)
  DoVistaChecks();
#endif

  g_target = new TargetCursor();
  EntityBlueprint::Initialise();
  g_windowManager->HideMousePointer();

  //
  // Start on a specific level if the prefs file tells us to

  const char* startMap = g_prefsManager->GetString("StartMap");
  if (startMap)
  {
    g_app->m_requestedLocationId = g_app->m_globalWorld->GetLocationId(startMap);
    GlobalLocation* gloc = g_app->m_globalWorld->GetLocation(g_app->m_requestedLocationId);
    ASSERT_TEXT(gloc, "Couldn't find location %s", startMap);
    strcpy(g_app->m_requestedMap, gloc->m_mapFilename);
    strcpy(g_app->m_requestedMission, gloc->m_missionFilename);
  }

  g_app->m_renderer->SetOpenGLState();

  DebugTrace("Inits 13: %f\n", GetHighResTime() - start);
  start = GetHighResTime();

  DebugTrace("Initialisation took: %f seconds\n", GetHighResTime() - startInit);
}

void Finalise()
{

  if (g_app->m_clientToServer)
    g_app->m_clientToServer->ClientLeave();

  if (g_app->m_soundSystem->IsInitialized() && g_soundLibrary2d)
  {
    g_soundLibrary2d->Stop();
    if (g_soundLibrary3d)
      delete g_soundLibrary3d;
    delete g_soundLibrary2d;
    g_soundLibrary3d = NULL;
    g_soundLibrary2d = NULL;
  }

  EclShutdown();

  if (!g_app->m_usingFontCopies)
    g_gameFont.Shutdown();
  else
    g_oldGameFont.Shutdown();

  if (!g_app->m_usingFontCopies)
    g_editorFont.Shutdown();
  else
    g_oldEditorFont.Shutdown();

  g_titleFont.Shutdown();

  delete g_windowManager;

  // Always clean on exit, this helps detect crashes and memory problems!
  //#ifdef TRACK_MEMORY_LEAKS	

  delete g_target;
  g_target = NULL;
  delete g_systemInfo;
  g_systemInfo = NULL;
  delete g_app;
  g_app = NULL;

  g_cachedSampleManager.EmptyCache();
  delete g_inputManager;
  g_inputManager = NULL;
  delete g_eventHandler;
  g_eventHandler = NULL;

  Profiler::Stop();
}

bool IsFirsttimeSequencing() { return g_RenderingFirstTimeBL; }

static void DoEnterLocation()
{
  bool iAmAServer = g_app->m_server != NULL;

  //
  // Start new task manager

  TaskManagerInterface::CreateTaskManager();

  double t = GetHighResTime();

  WorldObjectId::ResetUniqueIdSeed();
  bool loadingNew = true;
  g_app->m_location = new Location();

  g_app->m_locationInput = new LocationInput();

  if (loadingNew)
    g_app->m_location->Init(g_app->m_requestedMission, g_app->m_requestedMap);

  DebugTrace("Location Init: %f seconds\n", GetHighResTime() - t);

  GunTurret::SetupTeamShapes();

  g_app->m_locationId = g_app->m_requestedLocationId;

  g_app->m_camera->UpdateEntityTrackingMode();

  constexpr double borderSize = 200.0;
  double minX = -borderSize;
  double maxX = g_app->m_location->m_landscape.GetWorldSizeX() + borderSize;
  g_app->m_camera->SetBounds(minX, maxX, minX, maxX);
  g_app->m_camera->SetTarget(Vector3(maxX, 1000, maxX), Vector3(-1, -0.7, -1)); // In case start doesn't exist
  g_app->m_camera->SetTarget("start");
  g_app->m_camera->CutToTarget();
}

void EnterLocation()
{
  // m_loadingLocation is also set earlier when the game start message
  // is received, and prevents updates from being processed by the client
  g_app->m_loadingLocation = true;
  DebugTrace("Time at call to EnterLocation() = %f\n", GetHighResTime());

  g_loadingScreen->m_workQueue->Add(&DoEnterLocation);
  g_loadingScreen->Render();

  g_app->m_camera->SetDebugMode(Camera::DebugModeAuto);
  g_app->m_camera->RequestMode(Camera::ModeFreeMovement);

#ifdef BENCHMARK_AND_FTP
  g_app->m_benchMark->Begin();
#endif

  LocationGameLoop();

#ifdef BENCHMARK_AND_FTP
  g_app->m_benchMark->End(); g_app->m_benchMark->Upload();
#endif
}

void EnterGlobalWorld()
{
  if (g_app->m_gameMode == App::GameModePrologue && !g_app->m_script->IsRunningScript())
  {
    // the only time you should see the world in prologue is during the cutscene
    g_app->m_atMainMenu = true;
  }

  // Put the camera in a sensible place
  g_app->m_camera->SetDebugMode(Camera::DebugModeAuto);
  g_app->m_camera->RequestMode(Camera::ModeSphereWorld);
  g_app->m_camera->SetHeight(50.0);

  GlobalWorldGameLoop();
}

void MainMenuLoop()
{
  while (g_app->m_atMainMenu)
  {
    UpdateAdvanceTime();

    g_app->m_renderer->Render();
    g_app->m_userInput->Advance();

    if (g_app->m_soundSystem->IsInitialized())
      g_app->m_soundSystem->Advance();
    HandleCommonConditions();

    if (g_app->m_achievementTracker && !g_app->m_achievementTracker->HasLoaded())
      g_app->m_achievementTracker->Load();

    if (!g_app->m_gameMenu->m_menuCreated)
    {
      if (g_app->m_renderer->IsFadeComplete())
        g_app->m_gameMenu->CreateMenu();
    }

    //
    // Advance the server
    if (g_app->m_server)
      g_app->m_server->AdvanceIfNecessary(GetHighResTime());

    g_app->m_clientToServer->AdvanceAndCatchUp();
  }

  g_app->m_soundSystem->StopAllSounds(WorldObjectId(), "MultiwiniaInterface LobbyAmbience");
  g_app->m_gameMenu->DestroyMenu();
  g_app->m_requireSoundsLoaded = true;
}

void RunTheGame()
{
  Initialise();

  // 
  // Do whatever mode was requested

  g_loadingScreen->m_initialLoad = false;

  while (true)
  {
    if (requestBootloader)
    {
      requestBootloader = false;
    }

    if (g_app->m_atMainMenu)
      MainMenuLoop();
    else if (g_app->m_requestedLocationId != -1)
      EnterLocation();
    else // Not in location
      EnterGlobalWorld();

    g_inputManager->Advance();
  }
}

// Main Function
void AppMain(const char* _cmdLine) { RunTheGame(); }
