#ifndef _INCLUDED_APP_H
#define _INCLUDED_APP_H

#include "GameMain.h"
#include "Resource.h"
#include "multiwinia.h"
#include "net_lib.h"
#include "rgb_colour.h"

class Camera;
class Location;
class Server;
class ClientToServer;
class Renderer;
class UserInput;
class SoundSystem;
class LocationInput;
class LangTable;
class EffectProcessor;
class GlobalWorld;
class ParticleSystem;
class TaskManagerInterface;
class Script;
class MouseCursor;
class GameCursor;
class GameMenu;
class StartSequence;
class BitmapRGBA;
class Multiwinia;
class ShamanInterface;
class MarkerSystem;
class QNetManager;
class NetLib;

#ifdef BENCHMARK_AND_FTP
class BenchMark;
#endif
class AchievementTracker;
class WorkQueue;
class DelayedJob;
class TextReader;
class EntityGrid;
class Team;

#include "llist.h"

class GameApp : public GameMain
{
  public:
    GameApp();
    ~GameApp() override;

    void Startup() override;
    void Shutdown() override {}
    void Update([[maybe_unused]] float _deltaT) override {}
    void RenderScene() override {}

    void SetProfileName(const char* _profileName);
    bool LoadProfile();
    void ResetLevel(bool _global);

    void HandleDelayedJobs();
    void AddDelayedJob(DelayedJob* _dJob);

    void StartNetwork(bool _iAmAServer, const char* _serverIp, int _serverPort);
    HRESULT StartMultiPlayerServer();
    void ShutdownCurrentGame();

    std::string GetFirstAvailableLanguage();
    const char* GetDefaultLanguage();
    void InitLanguage();
    void SetLanguage(const char* _language, bool _test);
    bool TrySetLanguage(const std::string& _language);

    bool Multiplayer();
    bool IsSinglePlayer();

    static const char* GetProfileDirectory();
    static const char* GetPreferencesPath();
    static const char* GetMapDirectory();

    bool ToggleGamePaused();
    bool GamePaused() const;

    bool EarnedAchievement(int _achievementId);
    void GiveAchievement(int _achievementId);

    int GetMaxNumberofPlayers();
    bool UseChristmasMode();

    // Library Code Objects
    UserInput* m_userInput;
    Resource* m_resource;
    SoundSystem* m_soundSystem;
    ParticleSystem* m_particleSystem;
    LangTable* m_langTable;

    // Things that are the world
    GlobalWorld* m_globalWorld;
    Location* m_location;
    int m_locationId;

    // Everything else
    Camera* m_camera;
    Server* m_server; // Server process, can be NULL if client
    ClientToServer* m_clientToServer; // Clients connection to Server
    NetMutex m_networkMutex;
    NetMutex m_delayedJobListMutex;

    NetLib* m_netLib;

    NetThreadId m_mainThreadId;
    Renderer* m_renderer;
    LocationInput* m_locationInput;
    TaskManagerInterface* m_taskManagerInterface;
    Script* m_script;
    GameCursor* m_gameCursor;
    StartSequence* m_startSequence;
    GameMenu* m_gameMenu;
    Multiwinia* m_multiwinia;
    ShamanInterface* m_shamanInterface;
    MarkerSystem* m_markerSystem;
#ifdef BENCHMARK_AND_FTP
    BenchMark* m_benchMark;
#endif

    AchievementTracker* m_achievementTracker;

    int m_difficultyLevel; // Cached from preferences
    bool m_largeMenus;

    bool m_usingFontCopies;

    // State flags
    bool m_userRequestsPause;

    // Requested state flags
    int m_requestedLocationId; // -1 for global world
    bool m_requestQuit;

    char m_userProfileName[256];
    char m_requestedMission[256];
    char m_requestedMap[256];
    bool m_levelReset;
    char m_gameDataFile[256];

    RGBAColour m_backgroundColour;

    bool m_atMainMenu; // true when the player is viewing the darwinia/mutliwinia menu
    bool m_atLobby; // Viewing Lobby (Multiplayer Page)
    int m_gameMode;
    bool m_loadingLocation;
    bool m_spectator;

    bool m_hideInterface;
    bool m_soundsLoaded;
    bool m_requireSoundsLoaded;

    // these should be renamed once the list of achievements has been finalised
    enum
    {
      DarwiniaAchievementDominator = 0,
      // domnation achievement
      DarwiniaAchievementHoarder,
      // CTS achievement
      DarwiniaAchievementHailToTheKing,
      // king of the hill achievement
      DarwiniaAchievementUncladSkies,
      // rocket riot achievement
      DarwiniaAchievementAggravatedAssault,
      // Assault achievement
      DarwiniaAchievementMasterPlayer,
      // Beat someone who already has Master Player
      DarwiniaAchievementMasterOfMultiwinia,
      // won a game on every single game map
      DarwiniaAchievementExplorer,
      // played a game on every single map
      DarwiniaAchievementCarnage,
      // killed more than 3000 enemy darwinians in a single game
      DarwiniaAchievementBlitzMaster,
      // blitzkrieg achievement
      DarwiniaAchievementWrongGame,
      // nuke one of your allies in a 4p coop game
      DarwiniaAchievementGenocide,
      // Kill a total of 100,000 Darwinians
      NumAchievements
    };

    enum
    {
      GameModeNone,
      GameModePrologue,
      GameModeCampaign,
      GameModeMultiwinia,
      NumGameModes
    };

  private:
    WorkQueue* m_soundsWorkQueue;

    LList<DelayedJob*> m_delayedJobs;
};

extern GameApp* g_app;

#endif
