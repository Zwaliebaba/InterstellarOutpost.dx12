#ifndef __GAMEMENUWINDOW__
#define __GAMEMENUWINDOW__
// Game Menu Window

#include "game_menu.h"
#include "app.h"
#include "networkvalue.h"
#include "server.h"

class GameMenuInputField;
class AchievementButton;
struct ChatMessage;

class ServerPasswordString : public NetworkUnicodeString
{
  public:
    ServerPasswordString(const UnicodeString& _name = UnicodeString(), int _maxLength = 16)
      : m_name(_name),
        m_maxLength(_maxLength) {}

    const UnicodeString& Get() const override
    {
      if (g_app->m_server)
        return g_app->m_server->GetPassword();
      return m_name;
    }

    void Set(const UnicodeString& _x) override
    {
      m_name = _x;
      m_name.Truncate(m_maxLength);
      if (g_app->m_server)
        g_app->m_server->SetPassword(m_name);
    }

  private:
    UnicodeString m_name;
    int m_maxLength;
};

class GameMenuWindow : public DarwiniaWindow
{
  public:
    bool m_showingErrorDialogue;
    int m_currentPage;
    int m_newPage;
    bool m_bSpectator;
    int m_highlightedLevel; // used on the level page to show the level details of the currently highlighed map
    int m_localSelectedLevel;
    // a non-networked variable used to track level selection and prevent the game starting before the network value has been updated
    int m_helpPage;
    int m_currentHelpImage;
    int m_currentRequestedColour;
    int m_previousRequestedColour;
    int m_highlightedGameType;

    int m_uiGameType;
    int m_clickGameType;

    int m_screenTexture;

    // Multiwinia options
    NetworkGameOptionInt m_gameType;
    NetworkGameOptionMapId m_requestedMapId;
    NetworkGameOptionInt m_aiMode;
    NetworkGameOptionBool m_coopMode;
    NetworkGameOptionBool m_singleAssaultRound;
    NetworkGameOptionBool m_basicCrates;
    NetworkIntArray m_params;
    NetworkIntArray m_researchLevel;

    // Some parameters for the server browser
    LList<Directory*>* m_serverList;
    LList<AchievementButton*> m_achievementButtons;
    float m_serverX;
    float m_serverY;
    float m_serverW;
    float m_serverH;
    float m_serverGap;

    int m_serverButtonOrderStartIndex;

    Directory* m_joinServer; // Details on the server we are joining

    int m_xblaSessionType;
    int m_xblaSelectSessionTypeNextPage;
    int m_xblaSignInNextPage;
    bool m_xblaOfferSplitscreen;

    bool m_quickStart;
    bool m_singlePlayer;

    bool m_clickedCampaign;
    bool m_clickedPrologue;
    bool m_showingDeviceSelector;

    float m_darwiniaPlusAnimOffset;
    bool m_darwiniaPlusMPOnly;
    bool m_darwiniaPlusAnimating;
    float m_darwiniaPlusSelectionX, m_darwiniaPlusSelectionY, m_darwiniaPlusSelectionW, m_darwiniaPlusSelectionH;
    bool m_darwiniaPlusSelectedLeft;
    float m_darwiniaPlusSelectAnimTime;
    bool m_initedDarwiniaSelection;

    bool m_reloadPage;

    bool m_settingUpCustomGame;
    int m_customGameType;
    int m_customMapId;

    bool m_waitingForTeamID;
    int m_waitingGameType;

    PlayerNameString m_teamName;
    ServerPasswordString m_serverPassword;

    int m_gameTypeFilter;
    bool m_showDemosFilter;
    bool m_showIncompatibleGamesFilter;
    bool m_showFullGamesFilter;
    bool m_showCustomMapsFilter;
    bool m_showPasswordedFilter;
    bool m_motdFound;

    enum
    {
      PageMain = 0,
      PageDarwinia = 1,
      PageGameSelect = 2,
      PageNewOrJoin = 3,
      PageJoinGame = 4,
      PageQuickMatchGame = 5,
      PageGameSetup = 6,
      PageGameConnecting = 7,
      PageResearch = 8,
      PageMapSelect = 9,
      PageAdvancedOptions = 12,
      PagePlayerOptions = 24,
      PageSearchFilters = 25,
      PageUpdateAvailable = 31,
      PageCredits = 32,
      PageError = 34,
      PageAchievements = 35,
      PageEnterPassword = 36,
      PageWaitForGameToStart = 37,
      NumPages = 38
    };

    ScrollBar* m_scrollBar;

    LList<ChatMessage*> m_chatMessages;
    bool m_chatToggledThisUpdate;
    UnicodeString m_currentChatMessage;

    int m_serverIndex;

    bool m_disconnectFromGame;

  private:
    UnicodeString m_errorMessage;
    int m_errorBackPage;

    bool m_soundsStarted;
    bool m_setupAchievementsPage;

  public:
    GameMenuWindow();
    ~GameMenuWindow() override;

    void Create() override;
    void Update() override;
    void Render(bool _hasFocus) override;

    static void PreloadTextures();

    void SetupNewPage(int _page);

    void CreateErrorDialogue(UnicodeString _error, int _backPage = PageMain);

    static void RenderBigDarwinian();

    void NewChatMessage(UnicodeString _msg, int _fromTeamId);
    void SetTeamNameAndColour();

  private:
    friend class PlayGameButton;
    friend class QuickStartButton;
    friend class MapSelectModeButton;
    friend class ServerConnectButton;

    void UpdateMainPage();
    void UpdateGameOptions(bool _checkUserInterface);
    void UpdateJoinGamePage();
    void UpdateAchievementsPage();
    void UpdateGameConnectingPage();
    void UpdateMultiplayerPage();
    void UpdateQuickMatchGamePage();
    void UpdateAdvancedOptionsPage();
    void UpdateMapSelectPage();

    void RenderErrorDialogue();

    void SetupMainPage();
    void SetupNewOrJoinPage();
    void SetupJoinGamePage();
    void SetupAchievementsPage();
    void SetupDarwiniaPage();
    void SetupGameSelectPage();
    void SetupMultiplayerPage(int _gameType);
    void SetupGameConnectingPage();
    void SetupResearchPage();
    void SetupMapSelectPage();
    void SetupAdvancedOptionsPage();
    void SetupQuickMatchGamePage();
    void SetupPlayerOptionsPage();
    void SetupFiltersPage();
    void SetupUpdatePage();
    void SetupCreditsPage();
    void SetupErrorPage();
    void SetupPageEnterPassword();

    void ShutdownPage(int _page);
    void ShutdownJoinGamePage();
    void ShutdownAchievementsPage();

    void JoinServer(int _serverNumber);
    void JoinServerAsSpectator(int _serverNumber);

    GameMenuInputField* GameMenuWindow::CreateMenuControl(const char* name, int dataType, NetworkValue* value, int y, float change,
                                                          DarwiniaButton* callback, int x, int w, float fontSize);

    void GetDefaultPositions(int* _x, int* _y, int* _gap);

    void RenderNetworkStatus();

    void UpdateServerList();
    void ApplyServerFilters();

    void ApplyFilterProtocolMatch();
    void ApplyFilterGameType();
    void ApplyFilterFullGames();
    void ApplyFilterShowDemos();
    void ApplyFilterMissingMaps();
    void ApplyFilterInvalidGameTypes();
    void ApplyFilterPasswords();
    void ApplyBasicServerListSort();
    void ApplyBetterSlowerServerListSort();

    void PreloadThumbnails();

};

#endif
