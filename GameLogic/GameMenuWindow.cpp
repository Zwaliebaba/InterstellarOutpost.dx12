#include "pch.h"
#include "network_defines.h"
#include "app.h"
#include "renderer.h"
#include "preferences.h"
#include "resource.h"
#include "clienttoserver.h"
#include "servertoclient.h"
#include "server.h"
#include "soundsystem.h"
#include "unicode_text_stream_reader.h"
#include "MapData.h"
#include "game_menu.h"
#include "location.h"
#include "helpandoptions_windows.h"
#include "chatinput_window.h"
#include "GameMenuWindow.h"
#include "NewOrJoinButton.h"
#include "QuitButton.h"
#include "PrologueButton.h"
#include "WaitingButton.h"
#include "ServerTitleInfoButton.h"
#include "FiltersPageButton.h"
#include "NewServerButton.h"
#include "JoinGameButton.h"
#include "GameTypeInfoButton.h"
#include "GameTypeImageButton.h"
#include "GameTypeButton.h"
#include "FinishMultiwiniaButton.h"
#include "MapImageButton.h"
#include "MapSelectModeButton.h"
#include "MapInfoButton.h"
#include "PlayGameButton.h"
#include "AdvancedOptionsModeButton.h"
#include "GamerTagButton.h"
#include "MapListTitlesButton.h"
#include "AnyLevelSelectButton.h"
#include "LevelSelectButton.h"
#include "OptionDetailsButton.h"
#include "FancyColourButton.h"
#include "ApplyNameButton.h"
#include "AchievementButton.h"
#include "KickPlayerButton.h"
#include "ConnectButton.h"
#include "main.h"
#include "motdButton.h"
#include "update_page_buttons.h"
#include "mainmenus.h"

// Undefine small macro defined in Windows headers
#undef small

//#define	 FAKE_SERVERS
#define NUM_FAKE_SERVER 100

extern const char* s_pageNames[GameMenuWindow::NumPages];
extern int g_lastProcessedSequenceId;

const char* GetColourNameChar(int colourId)
{
  static char* names[8] = {"multiwinia_colour_green", "multiwinia_colour_red", "multiwinia_colour_yellow", "multiwinia_colour_blue",
                           "multiwinia_colour_orange", "multiwinia_colour_cyan", "multiwinia_colour_purple", "multiwinia_colour_pink"};

  return names[colourId];
}

UnicodeString GetColourName(int colourId)
{
  switch (colourId)
  {
  case 0:
    return LANGUAGEPHRASE("multiwinia_colour_green");
  case 1:
    return LANGUAGEPHRASE("multiwinia_colour_red");
  case 2:
    return LANGUAGEPHRASE("multiwinia_colour_yellow");
  case 3:
    return LANGUAGEPHRASE("multiwinia_colour_blue");
  case 4:
    return LANGUAGEPHRASE("multiwinia_colour_orange");
  case 6:
    return LANGUAGEPHRASE("multiwinia_colour_cyan");
  case 5:
    return LANGUAGEPHRASE("multiwinia_colour_purple");
  case 7:
    return LANGUAGEPHRASE("multiwinia_colour_pink");
  }

  return UnicodeString();
}

// *************************
// GameMenuWindow Class
// *************************

class GameMenuChatButton : public DarwiniaButton
{
  public:
    GameMenuChatButton()
      : DarwiniaButton() {}

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      auto parent = static_cast<GameMenuWindow*>(m_parent);

      static float boxHeight = 0.0f;
      float fontSize = m_fontSize;

      float x = realX;
      float y = realY + m_h;

      glColor4f(0.0f, 0.0f, 0.0f, 1.0f * (200.0f / 255.0f));
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      int h = m_h - fontSize * 1.5f;

      glBegin(GL_QUADS);
      glVertex2f(realX, realY);
      glVertex2f(realX + m_w, realY);
      glVertex2f(realX + m_w, realY + h);
      glVertex2f(realX, realY + h);
      glEnd();

      glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

      glBegin(GL_LINE_LOOP);
      glVertex2f(realX, realY);
      glVertex2f(realX + m_w, realY);
      glVertex2f(realX + m_w, realY + h);
      glVertex2f(realX, realY + h);
      glEnd();

      if (parent->m_chatMessages.Size() == 0)
      {
        return;
      }

      y -= (fontSize * 2.0f);

      int w = m_w * 0.95f;

      x += fontSize;

      boxHeight = fontSize;
      bool stopRendering = false;
      y -= (fontSize * 1.1f);
      for (int i = 0; i < parent->m_chatMessages.Size(); ++i)
      {
        float alphaTime = GetHighResTime() - parent->m_chatMessages[i]->m_recievedTime;
        bool maxed = false;
        if (!stopRendering)
        {
          float alpha = 255.0f;

          int teamId = -1;
          for (int j = 0; j < NUM_TEAMS; ++j)
          {
            if (g_app->m_multiwinia->m_teams[j].m_clientId == parent->m_chatMessages[i]->m_clientId && !g_app->m_multiwinia->m_teams[j].
              m_disconnected)
            {
              teamId = j;
              break;
            }
          }

          UnicodeString msg;
          UnicodeString teamName;
          if (teamId == -1)
            teamName = parent->m_chatMessages[i]->m_senderName;
          else
          {
            teamName = g_app->m_multiwinia->m_teams[teamId].m_teamName;
            parent->m_chatMessages[i]->m_colour = g_app->m_multiwinia->m_teams[teamId].m_colour;
            parent->m_chatMessages[i]->m_senderName = teamName;
          }
          if (teamName.Length() == 0)
          {
            teamName = UnicodeString("Player");
            parent->m_chatMessages[i]->m_colour = RGBAColour(150, 150, 150, 255);
          }
          UnicodeString thisMsg = parent->m_chatMessages[i]->m_msg;
          if (thisMsg == UnicodeString("PLAYERJOIN"))
          {
            msg = LANGUAGEPHRASE("multiwinia_player_join");
            msg.ReplaceStringFlag(L'P', teamName);
          }
          else if (thisMsg == UnicodeString("PLAYERLEAVE"))
          {
            msg = LANGUAGEPHRASE("multiwinia_player_leave");
            msg.ReplaceStringFlag(L'P', teamName);
          }
          else
            msg = teamName + UnicodeString(": ") + thisMsg;

          LList<UnicodeString*>* wrapped = WordWrapText(msg, m_w * 1.75f, fontSize, true, true);

          if (y < m_y + m_h * 0.05f)
            maxed = true;

          if (!maxed)
          {
            float a = alpha / 255.0f;
            for (int j = wrapped->Size() - 1; j >= 0; --j)
            {
              if (a == 1.0f)
              {
                g_editorFont.SetRenderOutline(true);
                glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
                g_editorFont.DrawText2D(x, y, fontSize, *wrapped->GetData(j));
              }
              RGBAColour col;
              if (teamId == -1)
                col = parent->m_chatMessages[i]->m_colour;
              else
              {
                col = g_app->m_multiwinia->m_teams[teamId].m_colour;
                parent->m_chatMessages[i]->m_colour = col;
              }
              col.a = alpha;
              glColor4ubv(col.GetData());
              g_editorFont.SetRenderOutline(false);
              g_editorFont.DrawText2D(x, y, fontSize, *wrapped->GetData(j));
              y -= fontSize * 1.1f;

              if (y < m_y + m_h * 0.05f)
              {
                maxed = true;
                break;
              }
            }
            boxHeight += (fontSize * 1.2f) * wrapped->Size();
          }
          wrapped->EmptyAndDelete();
          delete wrapped;
        }

        if (maxed)
        {
          // stop if we get too far up teh screen
          // we could break here, but then none of the older messages would ever get deleted
          stopRendering = true;
        }
      }
    }
};

GameMenuWindow::GameMenuWindow()
  : DarwiniaWindow("multiwinia_mainmenu_title"),
    m_showingErrorDialogue(false),
    m_currentPage(-1),
    m_newPage(PageMain),
    m_highlightedLevel(-1),
    m_localSelectedLevel(-1),
    m_helpPage(-1),
    m_currentHelpImage(-1),
    m_currentRequestedColour(-1),
    m_highlightedGameType(0),
    m_uiGameType(-1),
    m_gameType(GAMEOPTION_GAME_TYPE, g_app->m_multiwinia->m_gameType),
    m_aiMode(GAMEOPTION_AIMODE, g_app->m_multiwinia->m_aiType),
    m_coopMode(GAMEOPTION_COOPMODE, g_app->m_multiwinia->m_coopMode),
    m_singleAssaultRound(GAMEOPTION_SINGLEASSAULT, g_app->m_multiwinia->m_assaultSingleRound),
    m_basicCrates(GAMEOPTION_BASICCRATES, g_app->m_multiwinia->m_basicCratesOnly),
    m_params(MULTIWINIA_MAXPARAMS, NetworkGameParamFactory()),
    m_researchLevel(GlobalResearch::NumResearchItems, NetworkGameResearchLevelFactory()),
    m_serverList(nullptr),
    m_joinServer(nullptr),
    m_xblaSessionType(-1),
    m_xblaSelectSessionTypeNextPage(-1),
    m_xblaOfferSplitscreen(false),
    m_quickStart(false),
    m_clickedCampaign(false),
    m_clickedPrologue(false),
    m_showingDeviceSelector(false),
    m_darwiniaPlusAnimOffset(0.0f),
    m_darwiniaPlusMPOnly(false),
    m_darwiniaPlusAnimating(false),
    m_darwiniaPlusSelectionX(0.0f),
    m_darwiniaPlusSelectionY(0.0f),
    m_darwiniaPlusSelectionW(0.0f),
    m_darwiniaPlusSelectionH(0.0f),
    m_darwiniaPlusSelectedLeft(false),
    m_darwiniaPlusSelectAnimTime(0.0f),
    m_initedDarwiniaSelection(false),
    m_reloadPage(false),
    m_settingUpCustomGame(false),
    m_customGameType(-1),
    m_customMapId(-1),
    m_waitingForTeamID(false),
    m_waitingGameType(-1),
    m_gameTypeFilter(-1),
    m_showDemosFilter(true),
    m_showIncompatibleGamesFilter(true),
    m_showFullGamesFilter(true),
    m_showCustomMapsFilter(true),
    m_motdFound(false),
    m_scrollBar(nullptr),
    m_chatToggledThisUpdate(false),
    m_serverIndex(-1),
    m_disconnectFromGame(false),
    m_errorBackPage(-1),
    m_soundsStarted(false),
    m_setupAchievementsPage(false)
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();
  SetPosition(0, 0);
  SetSize(w, h);
  m_resizable = false;
  SetMovable(false);
  m_closeable = false;

  UnicodeString name;
  UnicodeString prefsName = g_prefsManager->GetUnicodeString("PlayerName", "multiwina_playername_default");
  bool defaultName = prefsName == UnicodeString("multiwina_playername_default");
  if (defaultName)
    name = LANGUAGEPHRASE("multiwina_playername_default");
  else
    name = UnicodeString(prefsName);

  for (int i = 0; i < wcslen(name.m_unicodestring); ++i)
    if (name.m_unicodestring[i] == L'%')
      name.m_unicodestring[i] = L' ';
  m_teamName.Set(name);

  ASSERT_TEXT(sizeof(s_pageNames)/sizeof(char *) == NumPages, "Mismatch between Page enum and static page names array, %d != %d",
              sizeof(s_pageNames)/sizeof(char *), NumPages);

  m_showDemosFilter = static_cast<bool>(g_prefsManager->GetInt("FilterShowDemos", 1));
  m_showIncompatibleGamesFilter = static_cast<bool>(g_prefsManager->GetInt("FilterShowIncompatible", 1));
  m_showFullGamesFilter = static_cast<bool>(g_prefsManager->GetInt("FilterShowFullGames", 1));
  m_showCustomMapsFilter = static_cast<bool>(g_prefsManager->GetInt("FilterShowCustomMaps", 1));
  m_showPasswordedFilter = static_cast<bool>(g_prefsManager->GetInt("FilterShowPassworded", 1));
}

GameMenuWindow::~GameMenuWindow() { m_chatMessages.EmptyAndDelete(); }

void GameMenuWindow::PreloadTextures() {}

void GameMenuWindow::Create() {}

void GameMenuWindow::Update()
{
  if (!m_soundsStarted)
  {
    if (!SoundSystem::NotReadyForGameSoundsYet())
    {
      m_soundsStarted = true;
      g_app->m_soundSystem->TriggerOtherEvent(nullptr, "LobbyAmbience", SoundSourceBlueprint::TypeMultiwiniaInterface);
    }
  }

  if (m_buttonOrder.Size() > 0 && !m_showingErrorDialogue && !ChatInputWindow::IsChatVisible() && !m_chatToggledThisUpdate)
    DarwiniaWindow::Update();
  else if (m_showingErrorDialogue)
  {
    if (g_inputManager->controlEvent(ControlCloseTaskHelp))
    {
      m_showingErrorDialogue = false;
      m_errorMessage = UnicodeString();
      m_lockMenu = false;
    }
  }

  if (m_currentPage != m_newPage)
  {
    ShutdownPage(m_currentPage);
    SetupNewPage(m_newPage);

    int w = g_app->m_renderer->ScreenW();
    int h = g_app->m_renderer->ScreenH();
    SetPosition(0, 0);
    SetSize(w, h);
    //}
  }
  else if (m_reloadPage)
  {
    SetupNewPage(m_currentPage);
    m_reloadPage = false;
  }

  switch (m_currentPage)
  {
  case PageMain:
    if (m_quickStart)
      UpdateGameOptions(false);
    UpdateMainPage();
    break;

  case PageJoinGame:
    UpdateJoinGamePage();
    break;

  case PageAchievements:
    UpdateAchievementsPage();
    break;

  case PageQuickMatchGame:
    UpdateQuickMatchGamePage();
    break;

  case PageGameConnecting:
    UpdateGameOptions(false);
    UpdateGameConnectingPage();
    break;

  case PageGameSetup:
    UpdateMultiplayerPage();
  case PageMapSelect:
  case PageGameSelect:
  case PagePlayerOptions:
    UpdateGameOptions(true);
    break;

  case PageAdvancedOptions:
    UpdateAdvancedOptionsPage();
    break;

  case PageWaitForGameToStart:
    break;
  }
}

void GameMenuWindow::CreateErrorDialogue(UnicodeString _error, int _backPage)
{
  m_errorMessage = _error;
  m_newPage = PageError;
  m_errorBackPage = _backPage;
  g_app->m_atLobby = false;

  m_chatMessages.EmptyAndDelete();
}

void GameMenuWindow::NewChatMessage(UnicodeString _msg, int _fromTeamId)
{
  auto message = new ChatMessage;
  message->m_msg = _msg;
  message->m_clientId = _fromTeamId;
  message->m_recievedTime = GetHighResTime();
  m_chatMessages.PutDataAtStart(message);

  for (int i = 0; i < NUM_TEAMS; ++i)
  {
    if (g_app->m_multiwinia->m_teams[i].m_clientId == _fromTeamId)
    {
      message->m_senderName = g_app->m_multiwinia->m_teams[i].m_teamName;
      message->m_colour = g_app->m_multiwinia->m_teams[i].m_colour;
      break;
    }
  }

  if (_msg != UnicodeString("PLAYERJOIN"))
    g_app->m_soundSystem->TriggerOtherEvent(nullptr, "ChatMessage", SoundSourceBlueprint::TypeMultiwiniaInterface);
}

void GameMenuWindow::UpdateMultiplayerPage()
{
  if (g_inputManager->controlEvent(ControlLobbyChat) && !EclIsTextEditing() && !m_chatToggledThisUpdate)
  {
    BeginTextEdit("chat_input");
    auto input = static_cast<ChatInputField*>(GetButton("chat_input"));
    input->m_buf = UnicodeString();
  }

  m_chatToggledThisUpdate = false;

  if (m_disconnectFromGame)
  {
    // we've clicked the exit game button but our team was set to ready
    // wait until our team is no longer ready before continuing
    bool go = false;
    for (int i = 0; i < NUM_TEAMS; ++i)
    {
      if (g_app->m_multiwinia->m_teams[i].m_teamType != TeamTypeUnused && !g_app->m_multiwinia->m_teams[i].m_ready)
      {
        go = true;
        break;
      }
    }
    int teamId = g_app->m_globalWorld->m_myTeamId;
    if (teamId != 255 && go)
    {
      if (!g_app->m_multiwinia->m_teams[teamId].m_ready)
      {
        g_app->ShutdownCurrentGame();
        g_app->m_atLobby = false;
        m_requestedMapId = -1;
        m_newPage = PageMain;

        m_chatMessages.EmptyAndDelete();
        m_serverPassword.Set(UnicodeString());

        m_disconnectFromGame = false;
      }
    }
  }

  if (g_app->m_server)
  {
    bool notReady = false;
    for (int i = 0; i < NUM_TEAMS; ++i)
    {
      LobbyTeam* team = &g_app->m_multiwinia->m_teams[i];
      if (team->m_teamType == TeamTypeUnused || team->m_teamType == TeamTypeCPU)
        continue;
      if (!team->m_ready)
      {
        notReady = true;
        break;
      }
    }
#ifdef TESTBED_ENABLED
    if (g_app->GetTestBedMode() == TESTBED_ON)
      notReady = !(g_app->GetTestBedState() == TESTBED_PLAY);
#endif

    if (!notReady)
    {
      if (0 <= m_gameType && m_gameType < MAX_GAME_TYPES && g_app->m_gameMenu->m_maps[m_gameType].ValidIndex(m_requestedMapId))
      {
        MapData* mapData = g_app->m_gameMenu->m_maps[m_gameType][m_requestedMapId];
        mapData->CalculeMapId();

        g_app->m_soundSystem->WaitIsInitialized();

        // fill up any empty player slots with AI teams before starting the game
        for (int i = 0; i < NUM_TEAMS; ++i)
        {
          if (i < g_app->m_gameMenu->m_maps[m_gameType][m_requestedMapId]->m_numPlayers && g_app->m_multiwinia->m_teams[i].m_teamType ==
            TeamTypeUnused)
            g_app->m_clientToServer->RequestTeam(TeamTypeCPU, -1);
        }

        UpdateGameOptions(true);
        g_app->m_clientToServer->RequestStartGame();

        // Transition to a new page while we wait for the game to start
        // instead of going through this again and again 
        m_newPage = PageWaitForGameToStart;
      }
      else
      {
        DebugTrace("Game Menu: Failed to start game due to invalid game type or map id (Game Type = %d, Map ID = %d)\n",
                   static_cast<int>(m_gameType), static_cast<int>(m_requestedMapId));
      }
    }
  }

  if (!m_waitingForTeamID)
    return;

  if (g_app->m_globalWorld->m_myTeamId != 255 || g_app->m_spectator)
  {
    DebugTrace("Setting up MP page\n");
    SetupMultiplayerPage(m_waitingGameType);
    m_waitingGameType = -1;
    m_waitingForTeamID = false;
  }
}

void GameMenuWindow::UpdateGameConnectingPage()
{
  if (g_app->m_clientToServer->m_connectionState == ClientToServer::StateConnected && m_gameType != -1)
  {
    //m_clickGameType = m_gameType;
    m_newPage = PageGameSetup;

    DebugTrace("------------------> Been disconnected\n");
  }
  else
  {
#ifdef TESTBED_ENABLED
    if (g_app->GetTestBedMode() == TESTBED_ON)
    {
      g_app->m_fTestBedDelay += GetHighResTime() - g_app->m_fTestBedLastTime;
      g_app->m_fTestBedLastTime = GetHighResTime();

      if (g_app->m_fTestBedDelay > 10.0f)
      {
        g_app->m_spectator = false;

        FinishMultiwiniaButton* pButton = (FinishMultiwiniaButton*)GetButton("multiwinia_menu_back");

        if (pButton) { pButton->MouseUp(); }

        //g_app->ShutdownCurrentGame();
        g_app->SetTestBedState(TESTBED_PICK_SERVER);
      }
    }
#endif

  }
}

void GameMenuWindow::SetTeamNameAndColour()
{
  ClientToServer* cToS = g_app->m_clientToServer;

  if (cToS)
  {
    int myTeam = g_app->m_globalWorld->m_myTeamId;
    if (myTeam != -1 && myTeam != 255 && m_currentRequestedColour != -1 && g_app->m_multiwinia->m_teams[myTeam].m_colourId !=
      m_currentRequestedColour && g_app->m_multiwinia->IsColourTaken(m_currentRequestedColour) && !m_coopMode && m_gameType !=
      Multiwinia::GameTypeAssault)
    {
      int preferedColour = g_prefsManager->GetInt("PlayerPreferedColour", -1);
      if (preferedColour != -1 && !g_app->m_multiwinia->IsColourTaken(preferedColour))
        m_currentRequestedColour = preferedColour;
      else
        m_currentRequestedColour = g_app->m_multiwinia->GetNextAvailableColour();
    }

    if (m_currentPage == PageGameSetup || m_currentPage == PagePlayerOptions)
    {
      if (m_currentRequestedColour != -1 || m_coopMode || m_gameType == Multiwinia::GameTypeAssault)
        cToS->ReliableSetTeamColour(m_currentRequestedColour);
    }

    if (!cToS->m_requestedTeamDetails.m_name && m_teamName.Get().WcsLen() > 0)
      cToS->ReliableSetTeamName(m_teamName.Get());
  }
}

void GameMenuWindow::UpdateGameOptions(bool _checkUserInterface)
{

  // Check to see if the user interface has updated any options
  ClientToServer* cToS = g_app->m_clientToServer;
  Multiwinia* mw = g_app->m_multiwinia;
  int teamId = g_app->m_globalWorld->m_myTeamId;

  //g_app->m_multiwinia->RemoveTeams(teamId);
  //g_app->m_clientToServer->RequestTeam(TeamTypeUnused,teamId);
  //g_app->m_spectator = true;

  if (_checkUserInterface && cToS)
    SetTeamNameAndColour();

  if (m_currentPage == PageMapSelect)
    UpdateMapSelectPage();

  bool gameTypeChanged = false;
  gameTypeChanged = m_uiGameType != mw->m_gameType && mw->m_gameType != -1 && m_uiGameType != -1;

  if (gameTypeChanged && (m_currentPage == PageGameSetup || m_currentPage == PageMapSelect))
  {
    ShutdownPage(m_currentPage);
    SetupNewPage(m_currentPage);
  }

  if (m_quickStart)
  {
    if (teamId != 255 || g_app->m_spectator)
    {
      g_app->m_soundSystem->WaitIsInitialized();

      //cToS->RequestTeam( TeamTypeLocalPlayer, 0 );
      SetTeamNameAndColour();

      if (m_gameType >= 0 && m_gameType < MAX_GAME_TYPES && g_app->m_gameMenu->m_maps[m_gameType].ValidIndex(m_requestedMapId))
      {
        for (int i = g_app->m_gameMenu->m_maps[m_gameType].GetData(m_requestedMapId)->m_numPlayers; i > 1; i--)
          cToS->RequestTeam(TeamTypeCPU, -1);
      }

      cToS->RequestStartGame();
      m_quickStart = false;
    }
  }
}

void GameMenuWindow::ApplyServerFilters()
{
  if (!m_serverList)
    return;

  ApplyFilterInvalidGameTypes();
  if (m_gameTypeFilter != -1)
    ApplyFilterGameType();

  if (!m_showFullGamesFilter)
    ApplyFilterFullGames();
  if (!m_showIncompatibleGamesFilter)
    ApplyFilterProtocolMatch();
  if (!m_showDemosFilter)
    ApplyFilterShowDemos();
  if (!m_showIncompatibleGamesFilter)
    ApplyFilterMissingMaps();
  if (!m_showPasswordedFilter)
    ApplyFilterPasswords();
}

void GameMenuWindow::ApplyBasicServerListSort()
{
  if (!m_serverList)
    return;

  auto newList = new LList<Directory*>();

  int numGoodServers = 0;

  for (int i = 0; i < m_serverList->Size(); ++i)
  {
    Directory* server = m_serverList->GetData(i);

    newList->PutDataAtStart(server);
    ++numGoodServers;
  }

  //
  // Replace the un-ordered list with the ordered list

  delete m_serverList;
  m_serverList = newList;
}

void GameMenuWindow::ApplyBetterSlowerServerListSort()
{
  if (!m_serverList)
    return;

  //
  // Score each server depending on the sort order

  for (int i = 0; i < m_serverList->Size(); ++i)
  {
    Directory* server = m_serverList->GetData(i);

    int score = 0;

    if (!IsProtocolMatch(server))
      score = 10;

    server->CreateData("OrderRank", score);
  }

  //
  // Build an ordered list based on the score

  auto newList = new LList<Directory*>();

  for (int i = 0; i < m_serverList->Size(); ++i)
  {
    Directory* server = m_serverList->GetData(i);
    int score = server->GetDataInt("OrderRank");

    bool added = false;
    for (int j = 0; j < newList->Size(); ++j)
    {
      Directory* thisServer = newList->GetData(j);
      int thisScore = thisServer->GetDataInt("OrderRank");

      if (score <= thisScore)
      {
        newList->PutDataAtIndex(server, j);
        added = true;
        break;
      }
    }

    if (!added)
      newList->PutDataAtEnd(server);
  }

  //
  // Replace the un-ordered list with the ordered list

  delete m_serverList;
  m_serverList = newList;

}

void GameMenuWindow::ApplyFilterPasswords()
{
  for (int i = 0; i < m_serverList->Size(); ++i)
  {
    Directory* d = m_serverList->GetData(i);
    if (d->HasData(NET_DARWINIA_SERVER_PASSWORD))
    {
      delete d;
      m_serverList->RemoveData(i);
      i--;
    }
  }
}

void GameMenuWindow::ApplyFilterGameType()
{
}

void GameMenuWindow::ApplyFilterFullGames()
{
}

void GameMenuWindow::ApplyFilterInvalidGameTypes()
{
}

void GameMenuWindow::ApplyFilterMissingMaps()
{
}

bool IsProtocolMatch(Directory* d)
{
  bool protocolMatch = true;
  return protocolMatch;
}

void GameMenuWindow::ApplyFilterProtocolMatch()
{
  for (int i = 0; i < m_serverList->Size(); ++i)
  {
    Directory* d = m_serverList->GetData(i);

    if (!IsProtocolMatch(d))
    {
      delete d;
      m_serverList->RemoveData(i);
      i--;
    }
  }
}

void GameMenuWindow::ApplyFilterShowDemos()
{
}

void GameMenuWindow::UpdateServerList()
{
}

void GameMenuWindow::JoinServer(int _serverNumber)
{
  // Signal that we are not joining as a spectator
  m_bSpectator = false;

  Directory* server = m_serverList->GetData(_serverNumber);

  if (m_joinServer)
    delete m_joinServer;
  m_joinServer = new Directory(*server);

  const char* serverIp = "10.0.0.127"; // server->GetDataString(NET_METASERVER_IP);
  int serverPort = 3001;               // server->HasData(NET_METASERVER_PORT) ? server->GetDataInt(NET_METASERVER_PORT) : -1;

  if (serverPort == -1)
    return;

  if (m_serverPassword.Get().Length() > 0)
    g_app->m_clientToServer->m_password = m_serverPassword.Get();

  g_app->StartNetwork(false, serverIp, serverPort);
  
  m_newPage = PageGameConnecting;
}

void GameMenuWindow::JoinServerAsSpectator(int _serverNumber)
{
  // Signal that we are joining as a spectator
  m_bSpectator = true;
  Directory* server = m_serverList->GetData(_serverNumber);

  if (m_joinServer)
    delete m_joinServer;
  m_joinServer = new Directory(*server);

  const char* serverIp = "10.0.0.127"; // server->GetDataString(NET_METASERVER_IP);
  int serverPort = 3001; // server->HasData(NET_METASERVER_PORT) ? server->GetDataInt(NET_METASERVER_PORT) : -1;

  g_app->StartNetwork(false, serverIp, serverPort);
  m_newPage = PageGameConnecting;
}

void GameMenuWindow::UpdateQuickMatchGamePage()
{
  double now = GetHighResTime();
  static double lastTimeUpdatedList = now;

  if (now > lastTimeUpdatedList + 1.0)
  {
    lastTimeUpdatedList = now;
    /*
    *
    * Check the metaserver for some games
    *
    */

    UpdateServerList();
  }

  if (m_serverList && m_serverList->Size() > 0)
  {
    // Found a game, let's try to join it
    for (int i = 0; i < m_serverList->Size(); i++)
    {
      if (m_serverList->ValidIndex(i))
      {
        Directory* d = m_serverList->GetData(i);

        JoinServer(i);
        break;
      }
    }
  }
}

void GameMenuWindow::UpdateMapSelectPage()
{
  if (m_clickGameType != -1)
  {
    if (m_clickGameType == m_gameType)
      SetupMapSelectPage();
  }
}

void GameMenuWindow::UpdateAdvancedOptionsPage()
{
  bool inactive = false;
  if (g_app->m_multiwinia->GetGameOption("TimeLimit") == 0)
    inactive = true;

  MultiwiniaGameBlueprint* blueprint = Multiwinia::s_gameBlueprints[m_gameType];
  for (int i = 0; i < blueprint->m_params.Size(); ++i)
  {
    MultiwiniaGameParameter* param = blueprint->m_params[i];
    if (strcmp(param->m_name, "SuddenDeath") == 0)
    {
      EclButton* button = GetButton(param->GetStringId());
      if (button)
      {
        if (button->m_inactive != inactive)
        {
          if (param->m_change == -1)
          {
            auto buttonCheckBox = static_cast<GameMenuCheckBox*>(button);
            buttonCheckBox->Set(inactive ? 0 : 1);
          }
          else if (param->m_change == 0)
          {
            auto buttonDropDown = static_cast<GameMenuDropDown*>(button);
            buttonDropDown->SelectOption(inactive ? 0 : 1);
          }
        }
        button->m_inactive = inactive;
      }
    }
  }
}

void GameMenuWindow::UpdateAchievementsPage()
{
  m_y = 0;
  if (g_app->m_achievementTracker->HasLoaded() && !m_setupAchievementsPage)
    m_setupAchievementsPage = true;
}

void GameMenuWindow::UpdateJoinGamePage()
{
  static double lastTimeUpdatedList = 0.0f;

  double now = GetHighResTime();
  bool updateNow = (now > lastTimeUpdatedList + 2.0f);
  bool receivedList = true;

  if (m_serverList && m_serverList->Size() == 0 && now > lastTimeUpdatedList + 1.0f && receivedList)
    updateNow = true;

  if (updateNow)
  {
    lastTimeUpdatedList = now;
    /*
    * Check the metaserver for some games
    */

    UpdateServerList();

    // Add in any manual server addresses
    for (int i = 0; i < 30; i++)
    {
      char prefsName[256];
      sprintf(prefsName, "Server%d", i + 1);

      if (!g_prefsManager->DoesKeyExist(prefsName))
        continue;

      const char* serverLine = g_prefsManager->GetString(prefsName, nullptr);
      if (!serverLine)
        continue;

      char serverHost[256] = "0.0.0.0";
      int serverPort = g_prefsManager->GetInt(PREFS_NETWORKSERVERPORT, 4000);
      char serverName[256]; // = "????";
      strcpy(serverName, serverLine);

      int numItems = sscanf(serverLine, "%s %d %s", serverHost, &serverPort, serverName);

      int gameMode = i % (Multiwinia::NumGameTypes - 1);
      auto d = new Directory;
      //d->CreateData(NET_METASERVER_SERVERNAME, serverName);
      //d->CreateData(NET_METASERVER_NUMTEAMS, static_cast<unsigned char>(0));
      //d->CreateData(NET_METASERVER_MAXTEAMS, static_cast<unsigned char>(4));
      //d->CreateData(NET_METASERVER_GAMEINPROGRESS, static_cast<unsigned char>(0));
      //d->CreateData(NET_METASERVER_GAMEMODE, static_cast<unsigned char>(gameMode));

      //d->CreateData(NET_METASERVER_IP, serverHost);
      //d->CreateData(NET_METASERVER_PORT, serverPort);
      //d->CreateData(NET_METASERVER_LOCALIP, serverHost);
      //d->CreateData(NET_METASERVER_LOCALPORT, serverPort);

      m_serverList->PutDataAtStart(d);
    }

    if (m_scrollBar)
      m_scrollBar->SetNumRows(m_serverList->Size());
  }
}

void GameMenuWindow::RenderNetworkStatus()
{
}

void GameMenuWindow::RenderBigDarwinian()
{
  //
  // Add colour to background

  glShadeModel(GL_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  float overscan = 0.0f;

  float x = 0;
  float y = 0;
  float w = g_app->m_renderer->ScreenW();
  float h = g_app->m_renderer->ScreenH();

  glBegin(GL_QUADS);
  glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
  glVertex2f(x - overscan, y - overscan);
  glVertex2f(x + w + overscan, y - overscan);
  glVertex2f(x + w + overscan, y + h + overscan);
  glColor4f(1.0f, 0.5f, 0.1f, 1.0f);
  glVertex2f(x - overscan, y + h + overscan);
  glEnd();

  //
  // Switch to lower quality rendering if our fps is bad

  static double s_smoothFps = 60.0f;
  static bool s_renderHighDetail = true;

  if (g_app->m_multiwinia->GameInLobby())
  {
    // Only make the decision if we're in the lobby
    // since in-game, other factors may destroy our frame rate
    double smoothFactor = g_advanceTime * 0.5f;
    s_smoothFps = (s_smoothFps * (1 - smoothFactor)) + static_cast<float>(g_app->m_renderer->m_fps * smoothFactor);
    if (s_smoothFps < 20)
      s_renderHighDetail = false;
    if (s_smoothFps >= 60)
      s_renderHighDetail = true;
  }

  //
  // Draw a big fucking darwinian

  Vector3 centrePos(x + w * 0.75f, 0, y + h * 0.7f);
  Vector3 up(0, 0, 1);
  Vector3 right(1, 0, 0);
  float size = w * 0.3f;

  //centrePos += Vector3( sinf(GetHighResTime()) * 5, 0, cosf(GetHighResTime()) + sinf(GetHighResTime()) * 4);
  size += sinf(GetHighResTime()) * 5;

  up.RotateAroundY(0.2f);
  right.RotateAroundY(0.2f);

  Vector3 pos1 = centrePos - up * size - right * size;
  Vector3 pos2 = centrePos - up * size + right * size;
  Vector3 pos3 = centrePos + up * size + right * size;
  Vector3 pos4 = centrePos + up * size - right * size;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("sprites/darwinian.bmp"));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
  glBegin(GL_QUADS);
  glTexCoord2i(0, 1);
  glVertex2f(pos1.x, pos1.z);
  glTexCoord2i(1, 1);
  glVertex2f(pos2.x, pos2.z);
  glTexCoord2i(1, 0);
  glVertex2f(pos3.x, pos3.z);
  glTexCoord2i(0, 0);
  glVertex2f(pos4.x, pos4.z);
  glEnd();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int numIterations = (s_renderHighDetail ? 10 : 0);

  for (int i = 0; i < numIterations; ++i)
  {
    size *= 1.02f;
    centrePos -= Vector3(6, 0, 6);
    pos1 = centrePos - up * size - right * size;
    pos2 = centrePos - up * size + right * size;
    pos3 = centrePos + up * size + right * size;
    pos4 = centrePos + up * size - right * size;

    float alpha = 0.2f * (1 - i / static_cast<float>(numIterations));
    glColor4f(1.0f, 0.2f, 0.2f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 1);
    glVertex2f(pos1.x, pos1.z);
    glTexCoord2i(1, 1);
    glVertex2f(pos2.x, pos2.z);
    glTexCoord2i(1, 0);
    glVertex2f(pos3.x, pos3.z);
    glTexCoord2i(0, 0);
    glVertex2f(pos4.x, pos4.z);
    glEnd();
  }

  glDisable(GL_TEXTURE_2D);
}

void RenderLoadingAchievementsEffect()
{
  float screenX = g_app->m_renderer->ScreenW() * 0.5f;
  float screenY = g_app->m_renderer->ScreenH() * 0.6f;

  float large, med, small;
  GetFontSizes(large, med, small);

  UnicodeString caption = LANGUAGEPHRASE("multiwinia_error_loadingachievement");

  glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
  g_editorFont.SetRenderOutline(true);
  g_editorFont.DrawText2DCentre(screenX, screenY, small, caption);

  if (fmodf(GetHighResTime(), 1.0f) < 0.5f)
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  else
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

  g_editorFont.SetRenderOutline(false);
  g_editorFont.DrawText2DCentre(screenX, screenY, small, caption);
}

void RenderRequestingEffect()
{
  float screenX = g_app->m_renderer->ScreenW() * 0.5f;
  float screenY = g_app->m_renderer->ScreenH() * 0.6f;

  float large, med, small;
  GetFontSizes(large, med, small);

  UnicodeString caption = LANGUAGEPHRASE("multiwinia_error_requestinglist");

  glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
  g_editorFont.SetRenderOutline(true);
  g_editorFont.DrawText2DCentre(screenX, screenY, small, caption);

  if (fmodf(GetHighResTime(), 1.0f) < 0.5f)
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  else
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

  g_editorFont.SetRenderOutline(false);
  g_editorFont.DrawText2DCentre(screenX, screenY, small, caption);
}

void GameMenuWindow::Render(bool _hasFocus)
{
  if (!_hasFocus && !EclGetWindow("location_chat"))
    return;

  RenderBigDarwinian();

  //
  // Render the title

  float titleX, titleY, titleW, titleH;
  float fontLarge, fontMed, fontSmall;

  GetPosition_TitleBar(titleX, titleY, titleW, titleH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  DrawFillBox(titleX, titleY, titleW, titleH);

  float fontX = titleX + titleW / 2;
  float fontY = titleY + titleH * 0.2f;

  UnicodeString titleCaption = LANGUAGEPHRASE("multiwinia_mainmenu_title");

  if (titleCaption.StrCmp("MULTIWINIA") == 0)
  {
    // Use the bitmap to render the title at higher quality

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("textures\\mwtitle.bmp", false));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    float titleImageW = g_titleFont.GetTextWidth(titleCaption.Length(), fontLarge);
    titleImageW *= 1.1f;
    float titleImageH = titleImageW / 5.4f;
    float titleImageX = titleX + titleW / 2.0f - titleImageW / 2.0f;
    float titleImageY = titleY + titleH * 0.1f;

    glBegin(GL_QUADS);
    glTexCoord2i(0, 1);
    glVertex2f(titleImageX, titleImageY);
    glTexCoord2i(1, 1);
    glVertex2f(titleImageX + titleImageW, titleImageY);
    glTexCoord2i(1, 0);
    glVertex2f(titleImageX + titleImageW, titleImageY + titleImageH);
    glTexCoord2i(0, 0);
    glVertex2f(titleImageX, titleImageY + titleImageH);
    glEnd();

    glDisable(GL_TEXTURE_2D);
  }
  else
  {
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    g_titleFont.SetRenderOutline(true);
    for (int i = 0; i < 3; ++i)
      g_titleFont.DrawText2DCentre(fontX, fontY, fontLarge, titleCaption);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    g_titleFont.SetRenderOutline(false);
    g_titleFont.DrawText2DCentre(fontX, fontY, fontLarge, titleCaption);
  }

  //
  // Render the subtitle

  glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
  g_titleFont.SetRenderOutline(true);
  g_titleFont.DrawText2DCentre(fontX + 5, fontY + fontLarge, fontMed, LANGUAGEPHRASE("multiwinia_mainmenu_subtitle"));
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  g_titleFont.SetRenderOutline(false);
  g_titleFont.DrawText2DCentre(fontX + 5, fontY + fontLarge, fontMed, LANGUAGEPHRASE("multiwinia_mainmenu_subtitle"));

  //
  // Darwinian logo

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("icons\\mwlogo.bmp"));
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  float screenW = g_app->m_renderer->ScreenW();
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  float logoRatio = 277.0f / 194.0f;
  float logoX = titleX + titleW * 0.05f;
  float logoY = titleY + titleH * 0.1f;
  float logoH = titleH * 0.8f;
  float logoW = logoH * logoRatio;

  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);
  glVertex2f(logoX, logoY);
  glTexCoord2f(1, 1);
  glVertex2f(logoX + logoW, logoY);
  glTexCoord2f(1, 0);
  glVertex2f(logoX + logoW, logoY + logoH);
  glTexCoord2f(0, 0);
  glVertex2f(logoX, logoY + logoH);
  glEnd();

  logoX = titleX + titleW - titleW * 0.05f - logoW;

  glBegin(GL_QUADS);
  glTexCoord2f(1, 1);
  glVertex2f(logoX, logoY);
  glTexCoord2f(0, 1);
  glVertex2f(logoX + logoW, logoY);
  glTexCoord2f(0, 0);
  glVertex2f(logoX + logoW, logoY + logoH);
  glTexCoord2f(1, 0);
  glVertex2f(logoX, logoY + logoH);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  //
  // Test render boxes

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;

  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);

  //
  // Render options
  // Build the help string as we go

  bool loadedAchievementImaged = true;
  switch (m_currentPage)
  {
  case PageGameSetup:
    DrawFillBox(leftX, leftY, leftW, leftH);
    DrawFillBox(rightX, rightY, rightW, rightH);
#ifdef TARGET_DEBUG
    RenderNetworkStatus();
#endif
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    break;

  case PageJoinGame:
    DrawFillBox(leftX, leftY, titleW, leftH);
    //DrawFillBox( rightX, rightY, rightW, rightH );

    if (!m_serverList || m_serverList->Size() == 0)
      RenderRequestingEffect();
    break;

  case PageAchievements:
    DrawFillBox(leftX, leftY, titleW, leftH);
    for (int i = 0; i < m_achievementButtons.Size(); i++)
      loadedAchievementImaged &= m_achievementButtons.GetData(i)->HasLoadedImage();
    if (!g_app->m_achievementTracker->HasLoaded() || !loadedAchievementImaged)
      RenderLoadingAchievementsEffect();
    break;

  case PageGameConnecting:
#ifdef _DEBUG
    RenderNetworkStatus();
#endif
    DrawFillBox(leftX, leftY, leftW, leftH);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    break;

  case PageGameSelect:
  case PageMapSelect:
  case PageAdvancedOptions:
    DrawFillBox(leftX, leftY, leftW, leftH);
    DrawFillBox(rightX, rightY, rightW, rightH);
    break;

  case PageMain:
  {
    DrawFillBox(leftX, leftY, leftW, leftH);

    float motdH = rightH;

    if (m_motdFound)
      DrawFillBox(rightX, rightY, rightW, motdH);
    break;
  }

  default:
    DrawFillBox(leftX, leftY, leftW, leftH);
    break;
  }

  //DarwiniaWindow::Render( _hasFocus );
  // render nothing but the buttons
  if (!GameMenuDropDownWindow::s_window)
    EclWindow::Render(_hasFocus);
  else
  {
    // If there is a DropDown menu open, only render that button.
    EclButton* button = GetButton(GameMenuDropDownWindow::s_window->m_name);
    if (button)
    {
      bool highlighted = EclMouseInButton(this, button) || strcmp(m_currentTextEdit, button->m_name) == 0;
      bool clicked = (_hasFocus && strcmp(EclGetCurrentClickedButton(), button->m_name) == 0);
      button->Render(m_x + button->m_x, m_y + button->m_y, highlighted, clicked);
    }
  }

  if (m_showingErrorDialogue)
    RenderErrorDialogue();
}

void GameMenuWindow::RenderErrorDialogue()
{
  int screenW = g_app->m_renderer->ScreenW();
  int screenH = g_app->m_renderer->ScreenH();

  int boxW = screenW / 3.0f;
  int boxH = screenH / 3.0f;
  int boxX = (screenW / 2.0f) - (boxW / 2.0f);
  int boxY = (screenH / 2.0f) - (boxH / 2.0f);

  DrawFillBox(boxX, boxY, boxW, boxH);

  float fontLarge, fontMed, fontSmall;
  GetFontSizes(fontLarge, fontMed, fontSmall);

  int yPos = boxY + fontSmall;
  int xPos = screenW / 2.0f;

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  g_titleFont.DrawText2DCentre(xPos, yPos, fontMed, LANGUAGEPHRASE("error"));

  yPos += fontMed * 1.5f;

  LList<UnicodeString*>* wrapped = WordWrapText(m_errorMessage, boxW * 1.5f, fontSmall);

  for (int i = 0; i < wrapped->Size(); ++i)
  {
    UnicodeString thisLine(*(wrapped->GetData(i)));
    g_titleFont.DrawText2DCentre(xPos, yPos += fontSmall * 1.1f, fontSmall, thisLine);
  }
  wrapped->EmptyAndDelete();
  delete wrapped;
}

void GameMenuWindow::SetupNewPage(int _page)
{
  DebugTrace("Setting up page: %s (%d) \n", s_pageNames[_page], _page);

  Remove();
  if (m_scrollBar)
  {
    delete m_scrollBar;
    m_scrollBar = nullptr;
  }

  switch (_page)
  {
  case PageMain:
    SetupMainPage();
    break;
  case PageDarwinia:
    SetupDarwiniaPage();
    break;
  case PageNewOrJoin:
    SetupNewOrJoinPage();
    break;
  case PageJoinGame:
    SetupJoinGamePage();
    break;
  case PageAchievements:
    SetupAchievementsPage();
    break;
  case PageGameSelect:
    SetupGameSelectPage();
    break;
  case PageGameConnecting:
    SetupGameConnectingPage();
    break;
  case PageGameSetup:
    SetupMultiplayerPage(m_gameType);
    break;
  case PageResearch:
    SetupResearchPage();
    break;
  case PageMapSelect:
    SetupMapSelectPage();
    break;
  case PageAdvancedOptions:
    SetupAdvancedOptionsPage();
    break;
  case PageQuickMatchGame:
    SetupQuickMatchGamePage();
    break;
  case PagePlayerOptions:
    SetupPlayerOptionsPage();
    break;
  case PageSearchFilters:
    SetupFiltersPage();
    break;
  case PageUpdateAvailable:
    SetupUpdatePage();
    break;
  case PageCredits:
    SetupCreditsPage();
    break;
  case PageError:
    SetupErrorPage();
    break;
  case PageEnterPassword:
    SetupPageEnterPassword();
    break;
  case PageWaitForGameToStart:
    break;
  default: ASSERT_TEXT(false, "Game menu invalid page (%d)", _page);
  }

  if (g_app->m_gameMenu->m_menuCreated && !m_reloadPage)
    g_app->m_renderer->StartMenuTransition();

  m_currentPage = _page;
}

void GameMenuWindow::SetupMainPage()
{
  SetTitle(LANGUAGEPHRASE("multiwinia_mainmenu_title"));

  m_settingUpCustomGame = false;
  m_localSelectedLevel = -1;

  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float rightX, rightY, rightW, rightH;
  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontMed;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_menu_mainmenutitle"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  // Multi player game

  GameMenuButton* button = new NewOrJoinButton("multiwinia_menu_multiplayergame");
  button->SetShortProperties("multiwinia_menu_multiplayergame", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                             LANGUAGEPHRASE("multiwinia_menu_multiplayergame"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  // Help and options

  button = new HelpAndOptionsButton();
  button->SetShortProperties("helpandoptions", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                             LANGUAGEPHRASE("multiwinia_menu_helpandoptions"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  yPos = leftY + leftH - buttonH * 2;

  // Quit

  button = new QuitButton();
  button->SetShortProperties("multiwinia_menu_quit", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_quit"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  auto motd = new MOTDButton();
  motd->SetShortProperties("multiwinia_motd", rightX + rightW * 0.05f, rightY + rightH * 0.05f, rightW * 0.9f, (rightH * 0.75f) * 0.9f,
                           UnicodeString());
  motd->m_fontSize = fontSmall;
  RegisterButton(motd);
}

void GameMenuWindow::SetupDarwiniaPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float x = leftX + (leftW - buttonW) / 2.0f;
  float y = leftY + buttonH;
  float fontSize = fontMed;

  auto pb = new PrologueButton("Prologue");
  pb->SetShortProperties("prologue", x, y, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_prologue"));
  pb->m_fontSize = fontSize;
  RegisterButton(pb);
  m_buttonOrder.PutData(pb);

  //auto cb = new CampaignButton("Campaign");
  //cb->SetShortProperties("campaign", x, y += gap + buttonH, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_campaign"));
  //RegisterButton(cb);
  //cb->m_fontSize = fontSize;
  //m_buttonOrder.PutData(cb);

  y = leftY + leftH - (buttonH * 2);

  auto back = new BackPageButton(PageMain);
  back->SetShortProperties("multiwinia_menu_back", x, y, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontSize;
  m_buttonOrder.PutData(back);
}

void GameMenuWindow::SetupQuickMatchGamePage()
{
  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float x = leftX + (leftW - buttonW) / 2.0f;
  float y = leftY;
  float fontSize = fontMed;

  float buttonX = x;

  y += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_setup_quick_match"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  y += buttonH * 1.3f;

  auto text = new WaitingButton("multiwinia_menu_searching");
  text->SetShortProperties("text", buttonX, y += buttonH + gap, buttonW, buttonH, UnicodeString());
  text->m_fontSize = fontSize;
  RegisterButton(text);

  m_serverX = buttonX;
  m_serverY = y;
  m_serverW = buttonW;
  m_serverH = buttonH * 0.6f;
  m_serverGap = m_serverH;
  m_serverButtonOrderStartIndex = m_buttonOrder.Size();

  float yPos = leftY + leftH - buttonH * 2;

  //SetTitle( LANGUAGEPHRASE( "multiwinia_setup_quick_match" ) );

  GameMenuButton* button = new BackPageButton(PageNewOrJoin);
  button->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);
}

void GameMenuWindow::SetupAchievementsPage()
{
  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float titleX, titleY, titleW, titleH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetPosition_TitleBar(titleX, titleY, titleW, titleH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float x = leftX + (leftW - buttonW) / 2.0f;
  float y = leftY + buttonH;
  float fontSize = fontMed;

  float buttonX = x;

  buttonW = (titleW / 2.0f) * 0.9f;

  m_serverX = buttonX;
  m_serverY = y;
  m_serverW = titleW * 0.9f;
  m_serverH = (g_app->m_renderer->ScreenH() / 16) * 1.6f; // 64 pixels is height of achievement images
  m_serverGap = m_serverH;
  m_serverButtonOrderStartIndex = m_buttonOrder.Size();

  float yPos = leftY;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, titleW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_menu_achievements"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.5f;

  m_serverY = yPos;

  int scrollbarW = buttonH / 2.0f;
  int scrollbarX = leftX + titleW - (scrollbarW + 10); // 10 + buttonW;
  int scrollbarY = m_serverY; //leftY+10+buttonH*1.6f;
  int scrollbarH = (leftY + leftH - buttonH * 2) - scrollbarY;

  m_scrollBar = new ScrollBar(this);
  m_scrollBar->Create("scrollBar", scrollbarX, scrollbarY, scrollbarW, scrollbarH, 0, 10);

  yPos = leftY + leftH - buttonH * 2;

  GameMenuButton* button = new BackPageButton(PageMain);
  button->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  m_setupAchievementsPage = false;
}

void GameMenuWindow::SetupJoinGamePage()
{
  m_highlightedGameType = -1;
  m_highlightedLevel = -1;

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float titleX, titleY, titleW, titleH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetPosition_TitleBar(titleX, titleY, titleW, titleH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float x = leftX + (leftW - buttonW) / 2.0f;
  float y = leftY + buttonH;
  float fontSize = fontMed;

  float buttonX = x;

  buttonW = (titleW / 2.0f) * 0.9f;

  m_serverX = buttonX;
  m_serverY = y;
  m_serverW = titleW * 0.9f;
  m_serverH = buttonH * 0.6f;
  m_serverGap = m_serverH;
  m_serverButtonOrderStartIndex = m_buttonOrder.Size();

  float yPos = leftY;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, titleW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_join_game_title"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  auto titles = new ServerTitleInfoButton();
  titles->SetShortProperties("Titles", buttonX, leftY + 10 + buttonH * 1.6f, m_serverW, buttonH * 0.6f, UnicodeString("Titles"));
  titles->m_fontSize = fontSmall;
  RegisterButton(titles);
  yPos += buttonH * 0.8f;

  m_serverY = yPos;

  int scrollbarW = buttonH / 2.0f;
  int scrollbarX = leftX + titleW - (scrollbarW + 10); // 10 + buttonW;
  int scrollbarY = m_serverY; //leftY+10+buttonH*1.6f;
  int scrollbarH = m_serverGap * SERVERS_ON_SCREEN;

  m_scrollBar = new ScrollBar(this);
  m_scrollBar->Create("scrollBar", scrollbarX, scrollbarY, scrollbarW, scrollbarH, 0, 10);

  yPos = leftY + leftH - buttonH * 2;

  GameMenuButton* button = new BackPageButton(PageNewOrJoin);
  button->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  button = new FiltersPageButton();
  button->SetShortProperties("filters_page", buttonX + buttonW * 1.1f, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_filters"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  //
  // Map preview on right side

  float thumbnailW = rightW * 0.8f;
  float thumbnailH = thumbnailW * 0.75f;
  float thumbnailX = rightX + rightW * 0.1f;

  yPos = rightY + buttonH;

  /*MapImageButton *mib = new MapImageButton();
  mib->SetShortProperties( "MapThumbnail", thumbnailX, yPos, thumbnailW, thumbnailH, UnicodeString("MapThumbnail") );
  RegisterButton( mib );

  MapInfoButton *info = new MapInfoButton();
  info->SetShortProperties( "MapInfo", thumbnailX, yPos + thumbnailH + fontSmall, thumbnailW, thumbnailH, UnicodeString("MapInfo") );
  info->m_fontSize = fontMed;
  RegisterButton( info );*/

#ifdef FAKE_SERVERS
  MetaServer_ClearServerList(); for (int i = 0; i < NUM_FAKE_SERVER; i++)
  {
    Directory* server = new Directory();
    char authkey[256];
    char name[256];
    char ip[16];
    char version[256];
    int port = rand() % 65000;
    int gameType = rand() % Multiwinia::NumGameTypes - 1; // No tank battle, please!
    int maxTeams = 1;
    int mapCRC = 0;
    int usedTeams;
    double time = GetHighResTime();

    MapData* mapData = NULL;

    // Determine how many players can join this game
    if (rand() % 5 == 1)
      gameType = -1;

    if (0 <= gameType && gameType < MAX_GAME_TYPES)
    {
      DArray<MapData*>& maps = g_app->m_gameMenu->m_maps[gameType];

      if (i > 0 && maps.ValidIndex(i % (((int)i / 6) + 1)))
      {
        MapData* m = maps[i % (((int)i / 6) + 1)];
        mapData = m;
        maxTeams = m->m_numPlayers;
        mapCRC = m->m_mapId;
      }
      else
      {
        for (int j = 0; j < maps.Size(); j++)
        {
          if (maps.ValidIndex(j))
          {
            MapData* m = maps[j];
            if (rand() % 500 <= 1)
            {
              mapData = m;
              maxTeams = m->m_numPlayers;
              mapCRC = m->m_mapId;
              break;
            }
          }
        }
      }
      // Didn't find a fake one, so use the first one we can
      if (!mapData)
      {
        for (int j = 0; j < maps.Size(); j++)
        {
          if (maps.ValidIndex(j))
          {
            MapData* m = maps[j];
            mapData = m;
            maxTeams = m->m_numPlayers;
            mapCRC = m->m_mapId;
            break;
          }
        }
      }
    }

    usedTeams = (rand() % maxTeams) + 1;
    sprintf(name, "FakeServer-%d", i);
    sprintf(ip, "%d.%d.%d.%d", rand() % 255, rand() % 255, rand() % 255, rand() % 255);
    if (rand() % 2 == 1)
      sprintf(version, "1.9.%d", i % 16);
    else
      sprintf(version, APP_VERSION);

    Authentication_GenerateKey(authkey, 4, true);

    server->CreateData(NET_METASERVER_SERVERNAME, name);
    server->CreateData(NET_METASERVER_GAMENAME, APP_NAME);
    server->CreateData(NET_METASERVER_GAMEVERSION, version);
    server->CreateData(NET_METASERVER_STARTTIME, *(unsigned long long*)&time);
    server->CreateData(NET_METASERVER_RANDOM, i);
    server->CreateData(NET_METASERVER_LOCALIP, ip);
    server->CreateData(NET_METASERVER_LOCALPORT, port);
    server->CreateData(NET_METASERVER_NUMTEAMS, (unsigned char)usedTeams);
    server->CreateData(NET_METASERVER_NUMHUMANTEAMS, (unsigned char)(rand() % (usedTeams + 1)));
    server->CreateData(NET_METASERVER_MAXTEAMS, (unsigned char)maxTeams);
    server->CreateData(NET_METASERVER_NUMSPECTATORS, (unsigned char)0);
    server->CreateData(NET_METASERVER_MAXSPECTATORS, (unsigned char)0);
    server->CreateData(NET_METASERVER_GAMEINPROGRESS, (unsigned char)((bool)(rand() % 2)));
    server->CreateData(NET_METASERVER_GAMEMODE, (unsigned char)gameType);
    server->CreateData(NET_METASERVER_MAPCRC, mapCRC);
    server->CreateData(NET_METASERVER_AUTHKEY, authkey);
    server->CreateData(NET_METASERVER_PORT, port);

    MetaServer_UpdateServerList(ip, port, true, server);
  }
#endif
}

void GameMenuWindow::ShutdownPage(int _page)
{
  switch (_page)
  {
  case PageJoinGame:
    ShutdownJoinGamePage();
    break;

  case PageGameConnecting:
    break;

  case PageAchievements:
    ShutdownAchievementsPage();
    break;
  }
}

void GameMenuWindow::ShutdownAchievementsPage() { m_achievementButtons.Empty(); }

void GameMenuWindow::ShutdownJoinGamePage()
{
}

void GameMenuWindow::UpdateMainPage()
{
}

void GameMenuWindow::SetupNewOrJoinPage()
{
  GameMenuButton* button = nullptr;
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2;
  float yPos = leftY;
  float fontSize = fontMed;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_neworjoin_title"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  m_xblaSessionType = -1;
  m_settingUpCustomGame = false;

  button = new NewServerButton();
  button->SetShortProperties("multiwinia_menu_hostgame", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                             LANGUAGEPHRASE("multiwinia_menu_hostgame"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  button = new JoinGameButton();
  button->SetShortProperties("multiwinia_menu_joingame", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                             LANGUAGEPHRASE("multiwinia_menu_joingame"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  yPos = leftY + leftH - buttonH * 2;

  button = new BackPageButton(PageMain);
  button->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);
}

void GameMenuWindow::SetupGameSelectPage()
{
  if (!g_app->m_server)
  {
    HRESULT hr;
    hr = g_app->StartMultiPlayerServer();
  }

  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float xPos = buttonX;
  float yPos = leftY;
  float fontSize = fontMed;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_gameselect_title"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  for (int i = 0; i < Multiwinia::s_gameBlueprints.Size(); ++i)
  {
    auto button = new GameTypeButton(Multiwinia::s_gameBlueprints[i]->GetName(), i);
    button->SetShortProperties(Multiwinia::s_gameBlueprints[i]->GetName(), xPos, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE(Multiwinia::s_gameBlueprints[i]->GetName()));
    button->m_fontSize = fontSize;
    if (strcmp(button->m_name, "TankBattle") == 0)
      button->m_inactive = true;

    if (!button->m_inactive)
    {
      // LEANDER : This hides tank battle from the menu until it is unlocked. 
      // This would be the place to show some graphic about the tank and which PDLCs you have.
      m_buttonOrder.PutData(button);
      RegisterButton(button);
    }
  }

  int page = PageNewOrJoin;
  m_localSelectedLevel = -1;

  yPos = leftY + leftH - buttonH * 2;
  GameMenuButton* button = new FinishMultiwiniaButton(page);
  button->SetShortProperties("multiwinia_menu_back", xPos, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  //
  // Status buttons on the right

  xPos = rightX + rightW * 0.1f;
  yPos = rightY + rightW * 0.1f;
  int thumbnailW = rightW * 0.8f;
  int thumbnailH = thumbnailW * 2 / 3.0f;

  auto imageButton = new GameTypeImageButton();
  imageButton->SetShortProperties("gametype", xPos, yPos, thumbnailW, thumbnailH, UnicodeString("gametype"));
  RegisterButton(imageButton);

  auto info = new GameTypeInfoButton();
  info->SetShortProperties("gametypeinfo", xPos, yPos + thumbnailH + fontSmall, thumbnailW, (rightH * 0.85f) - thumbnailH, UnicodeString());
  info->m_fontSize = fontSmall;
  RegisterButton(info);
}

void GameMenuWindow::SetupGameConnectingPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float xPos = buttonX;
  float yPos = leftY;
  float fontSize = fontMed;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_menu_joining"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  auto waiting = new WaitingButton("multiwinia_menu_connecting");
  waiting->SetShortProperties("connecting", xPos, yPos += buttonH + gap, buttonW, buttonH, UnicodeString());
  waiting->m_fontSize = fontSize;
  RegisterButton(waiting);

  int page = -1;
  if (m_currentPage == PageJoinGame)
    page = PageJoinGame;
  else
    page = PageNewOrJoin;

  yPos = leftY + leftH - buttonH * 2;
  GameMenuButton* button = new FinishMultiwiniaButton(page);
  button->SetShortProperties("multiwinia_menu_back", xPos, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  button->m_fontSize = fontSize;
  RegisterButton(button);
  m_buttonOrder.PutData(button);
}

void GameMenuWindow::SetupMultiplayerPage(int _gameType)
{
  if (_gameType == -1)
    return;
  int mapId = m_localSelectedLevel;
  if (mapId == -1)
    mapId = m_requestedMapId;
  if (mapId == -1 || (g_app->m_globalWorld->m_myTeamId == 255 && !g_app->m_spectator))
  {
    m_waitingForTeamID = true;
    m_waitingGameType = _gameType;
    return;
  }

  if (m_currentPage == PageMapSelect)
  {
    if (m_uiGameType != _gameType)
    {
      SetupNewPage(PageMapSelect);
      return;
    }
  }
  else if (m_currentPage != PageResearch && m_currentPage != PageAdvancedOptions && m_currentPage != PagePlayerOptions)
  {
    m_uiGameType = _gameType;

    if (g_app->m_server)
      m_requestedMapId = 0;
  }

  g_app->m_atLobby = true;

  MultiwiniaGameBlueprint* blueprint = Multiwinia::s_gameBlueprints[_gameType];

  if (m_currentPage == PagePlayerOptions && m_teamName.Get().WcsLen() == 0)
    m_teamName.Set(LANGUAGEPHRASE("multiwina_playername_default"));

  if (m_currentPage != PagePlayerOptions && m_currentPage != PageAdvancedOptions)
  {
    if (_gameType == Multiwinia::GameTypeAssault || m_coopMode)
    {
      m_previousRequestedColour = m_currentRequestedColour;
      m_currentRequestedColour = -1;
    }
    else
    {
      int preferedColour = g_prefsManager->GetInt("PlayerPreferedColour", -1);
      if (preferedColour != -1 && !g_app->m_multiwinia->IsColourTaken(preferedColour))
        m_currentRequestedColour = preferedColour;
      else
        m_currentRequestedColour = g_app->m_multiwinia->GetNextAvailableColour();
    }
  }

  m_highlightedLevel = -1;

  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float xPos = buttonX;
  float yPos = leftY + buttonH;
  float fontSize = fontSmall;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE(blueprint->GetName()));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  //
  // Map thumbnail + info on right side

  float thumbnailW = rightW * 0.8f;
  float thumbnailH = thumbnailW * 0.75f;
  float thumbnailX = rightX + rightW * 0.1f;
  float thumbnailY = rightY + rightW * 0.1f;

  auto mib = new MapImageButton();
  mib->SetShortProperties("MapThumbnail", thumbnailX, thumbnailY, thumbnailW, thumbnailH, UnicodeString("MapThumbnail"));
  RegisterButton(mib);
  thumbnailY += thumbnailH + fontSmall * 0.5f;

  auto msmb = new MapSelectModeButton();
  msmb->SetShortProperties("Map", thumbnailX, thumbnailY, thumbnailW, buttonH * 0.66f, " ");
  msmb->m_fontSize = fontSmall;
  RegisterButton(msmb);

  thumbnailY += buttonH * 1.0f + gap;

  float infoH = rightH - thumbnailH - (fontSmall * 0.5f) - 2.0f * (buttonH + gap) - gap;

  float font = fontSmall;
  font *= 0.8f;
  DarwiniaButton* info = new GameMenuChatButton();
  info->SetShortProperties("chat_button", thumbnailX, thumbnailY, thumbnailW, infoH, UnicodeString("Chat"));
  info->m_fontSize = font;
  RegisterButton(info);

  auto chatwindow = new ChatInputField();
  chatwindow->SetShortProperties("chat_input", rightX + rightW * 0.1f, rightY + (rightH * 0.95f), rightW * 0.8f, fontSmall * 1.2f,
                                 UnicodeString());
  chatwindow->RegisterUnicodeString(m_currentChatMessage);
  RegisterButton(chatwindow);

  if (g_app->m_server)
  {
    if (m_currentPage != PageResearch &&
      m_currentPage != PageAdvancedOptions && m_currentPage != PagePlayerOptions)
    {
      for (int i = 0; i < blueprint->m_params.Size(); ++i)
      {
        MultiwiniaGameParameter* param = blueprint->m_params[i];
        m_params[i] = param->m_default;
      }
    }
  }

  UnicodeString caption = LANGUAGEPHRASE("multiwinia_menu_ready");

  auto play = new PlayGameButton();
  play->SetShortProperties("Play", xPos, yPos, buttonW, buttonH, caption);
  RegisterButton(play);
  play->m_fontSize = fontMed;
  m_buttonOrder.PutData(play);
  m_currentButton = m_buttonOrder.Size() - 1;

  yPos += buttonH + gap;

  if (g_app->m_server)
  {

#if !defined(SPECTATOR_ONLY)
    auto aomb = new AdvancedOptionsModeButton();
    aomb->SetShortProperties("Advanced Options", xPos, yPos, buttonW, buttonH * 0.66f, LANGUAGEPHRASE("multiwinia_menu_advanceoptions"));
    aomb->m_fontSize = fontSmall;
    RegisterButton(aomb);
    m_buttonOrder.PutData(aomb);
    yPos += buttonH * 0.66f + gap;

    auto menu = new GameMenuDropDown();
    menu->SetShortProperties("multiwinia_menu_aimode", xPos, yPos, buttonW, buttonH * 0.66f, LANGUAGEPHRASE("multiwinia_menu_aimode"));
    menu->AddOption("dialog_easy_difficulty", Multiwinia::AITypeEasy);
    menu->AddOption("multiwinia_menu_normal", Multiwinia::AITypeStandard);
    menu->AddOption("multiwinia_menu_hard", Multiwinia::AITypeHard);
    RegisterButton(menu);
    menu->RegisterNetworkInt(&(m_aiMode));
    menu->m_fontSize = fontSize;
    menu->m_noBackground = true;
    menu->m_cycleMode = true;
    m_buttonOrder.PutData(menu);
#endif
  }
  else
  {
    yPos += buttonH + gap;
    yPos += buttonH * 0.66f + gap;
  }

  //
  // Game type image button

  float gameTypeW = buttonW * 0.45f;
  float gameTypeH = gameTypeW * 2 / 3.0f;
  float gameTypeX = leftX + leftW - gameTypeW - (leftW - buttonW) / 2.0f;
  float gameTypeY = leftY + buttonH * 6;

  auto imageButton = new GameTypeImageButton();
  imageButton->SetShortProperties("gametype", gameTypeX, gameTypeY, gameTypeW, gameTypeH, UnicodeString("gametype"));
  RegisterButton(imageButton);

  //
  // Back button

  BackPageButton* back;
  if (g_app->m_server)
    back = new FinishMultiwiniaButton(PageGameSelect);
  else
    back = new FinishMultiwiniaButton(PageNewOrJoin);

  yPos = leftY + leftH - buttonH * 2;
  back->SetShortProperties("multiwinia_menu_back", xPos, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;

  buttonX = leftX + (leftW - buttonW) / 2.0f;
  yPos = leftY + buttonH * 6;
  fontSize = fontSmall;
  buttonH *= 0.75f;
  buttonW *= 0.45f;

  int gameType = _gameType;

  ASSERT_TEXT(0 <= gameType && gameType < MAX_GAME_TYPES && g_app->m_gameMenu->m_maps[gameType].ValidIndex( mapId ),
              "Failed to get level due to invalid game type or map id (Game Type = %d, Map ID = %d), map name (may be inaccurate) = '%s'",
              gameType, mapId, g_app->m_requestedMap);
  MapData* md = g_app->m_gameMenu->m_maps[gameType][mapId];

  for (int i = 0; i < md->m_numPlayers; ++i)
  {
    auto gtb = new GamertagButton();
    char tagname[64];
    sprintf(tagname, "tag%d", i);
    gtb->SetShortProperties(tagname, buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("teamtype_2")); // TeamTypeCPU
    gtb->m_fontSize = fontSize;
    gtb->m_teamId = i;
    RegisterButton(gtb);
    m_buttonOrder.PutData(gtb);

    if (i > 0 && g_app->m_server)
    {
      char kickname[64];
      sprintf(kickname, "kick%d", i);
      auto kick = new KickPlayerButton();
      kick->SetShortProperties(kickname, buttonX + buttonW * 1.05f, yPos, buttonH, buttonH, LANGUAGEPHRASE("teamtype_2")); // TeamTypeCPU
      kick->m_teamId = i;
      RegisterButton(kick);
      m_buttonOrder.PutData(kick);
    }

    yPos += buttonH * 1.5f;
  }

  m_buttonOrder.PutData(back);

}

void GameMenuWindow::SetupResearchPage()
{
  int x, y, gap;
  GetDefaultPositions(&x, &y, &gap);
  int w = 600 * static_cast<float>(x / 200.0f);
  int h = (w / 12.0f);
  float fontSize = 0.9f * h;
  if (h - fontSize > gap)
    h = fontSize;

  y -= gap;

  for (int i = 0; i < GlobalResearch::NumResearchItems; ++i)
  {
    char* name = GlobalResearch::GetTypeName(i); //Translated( i );
    m_researchLevel[i].SetBounds(0, 4);
    CreateMenuControl(name, InputField::TypeInt, &(m_researchLevel[i]), y += gap, 1, nullptr, x, w, fontSize);
    GetButton(name)->SetCaption(name);
    GetButton(name)->m_h = h;
    m_buttonOrder.PutData(GetButton(name));
  }

  auto back = new BackPageButton(PageGameSetup);
  back->SetShortProperties("multiwinia_menu_back", x, y += gap * 1.5f, w, h, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontSize * 1.2f;
  m_buttonOrder.PutData(back);
}

void GameMenuWindow::PreloadThumbnails()
{
  DebugTrace("GameMenuWindow - Starting to preload thumbnails.\n");
  int gameType = m_settingUpCustomGame ? m_customGameType : m_gameType;

  if (gameType < 0 || gameType >= MAX_GAME_TYPES)
    return;

  DArray<MapData*>& maps = g_app->m_gameMenu->m_maps[gameType];

  for (int i = 0; i < maps.Size(); ++i)
  {
    if (!maps.ValidIndex(i))
      continue;

    char thumbfile[256];
    char fullFilename[256];
    strcpy(thumbfile, maps[i]->m_fileName);
    thumbfile[strlen(thumbfile) - 4] = '\0';
    strcat(thumbfile, "." TEXTURE_EXTENSION);
    sprintf(fullFilename, "thumbnails\\%s", thumbfile);
    strlwr(fullFilename);

    if (g_app->m_resource->DoesTextureExist(fullFilename))
      g_app->m_resource->GetTexture(fullFilename, false, false);
  }
}

void GameMenuWindow::SetupMapSelectPage()
{
  DebugTrace("Begin Caching Thumbnails - %f\n", GetHighResTime());
  g_loadingScreen->m_workQueue->Add(&GameMenuWindow::PreloadThumbnails, this);
  DebugTrace("Starting rendering the loading screen - %f\n", GetHighResTime());
  g_loadingScreen->Render();
  DebugTrace("End Caching Thumbnails - %f\n", GetHighResTime());

  if (m_clickGameType != -1 && m_gameType != m_clickGameType && !m_settingUpCustomGame)
    return;
  int gameType = m_gameType;
  if (m_settingUpCustomGame)
    gameType = m_customGameType;
  if (gameType < 0 || gameType >= MAX_GAME_TYPES)
    return;
  m_uiGameType = gameType;
  m_localSelectedLevel = -1;
  m_clickGameType = -1;

  MultiwiniaGameBlueprint* blueprint = Multiwinia::s_gameBlueprints[gameType];

  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float xPos = buttonX;
  float yPos = leftY;
  float fontSize = fontSmall;

  float thumbnailW = rightW * 0.8f;
  float thumbnailH = thumbnailW * 0.75f;
  float thumbnailX = rightX + rightW * 0.1f;
  float thumbnailY = rightY + rightW * 0.1f;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE(blueprint->GetName()));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  //
  // Subtitle

  auto titles = new MapListTitlesButton();
  titles->SetShortProperties("Titles", xPos, leftY + 10 + buttonH * 1.8f, buttonW, buttonH * 0.7f, UnicodeString("Titles"));
  titles->m_fontSize = fontSmall;
  titles->m_screenId = PageMapSelect;
  RegisterButton(titles);
  yPos += buttonH * 0.35f;

  auto mib = new MapImageButton();
  mib->SetShortProperties("MapThumbnail", thumbnailX, thumbnailY, thumbnailW, thumbnailH, UnicodeString("MapThumbnail"));
  RegisterButton(mib);

  auto info = new MapInfoButton();
  info->SetShortProperties("MapInfo", thumbnailX, thumbnailY + thumbnailH + fontSmall, thumbnailW, (rightH * 0.85f) - thumbnailH,
                           UnicodeString("MapInfo"));
  info->m_fontSize = fontSmall * 1.1f;
  RegisterButton(info);

  DArray<MapData*>& maps = g_app->m_gameMenu->m_maps[gameType];

  if (m_settingUpCustomGame)
  {
    auto lsb = new AnyLevelSelectButton;
    auto name = "multiwinia_level_any";

    lsb->SetProperties(name, xPos, yPos += buttonH * 0.6f, buttonW, buttonH * 0.6f, LANGUAGEPHRASE(name));
    lsb->m_fontSize = fontSize;

    RegisterButton(lsb);
    m_buttonOrder.PutData(lsb);
  }

  float newButtonX = leftX + ((buttonX - leftX) / 2);
  float newButtonW = buttonW; // + ( (leftW - buttonW)/2 );

  for (int i = 0; i < maps.Size(); ++i)
  {
    auto lsb = new LevelSelectButton();

    char name[512];
    if (maps[i]->m_levelName)
      strcpy(name, maps[i]->m_levelName);
    else
      strcpy(name, maps[i]->m_fileName);

    float levelSButtonH = buttonH * 0.7f;
    float levelSButtonG = (buttonH + gap) * 0.7f;
    lsb->SetProperties(name, xPos, yPos += levelSButtonG, newButtonW, levelSButtonH, LANGUAGEPHRASE(name));
    lsb->m_levelIndex = i;
    lsb->m_fontSize = fontSmall;
    lsb->m_inactive = false;

    RegisterButton(lsb);
    m_buttonOrder.PutData(lsb);
  }

  yPos = leftY + leftH - buttonH * 2;

  int page = PageGameSelect;
  if (m_requestedMapId == -1)
    page = PageGameSelect;
  auto back = new BackPageButton(page);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);

}

void GameMenuWindow::SetupAdvancedOptionsPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float rightX, rightY, rightW, rightH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetPosition_RightBox(rightX, rightY, rightW, rightH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float xPos = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontSmall;

  //
  // Title

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_menu_advanceoptions"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  buttonH *= 0.65f;

  if (m_gameType != -1)
  {
    MultiwiniaGameBlueprint* blueprint = Multiwinia::s_gameBlueprints[m_gameType];
    for (int i = 0; i < blueprint->m_params.Size(); ++i)
    {
      MultiwiniaGameParameter* param = blueprint->m_params[i];
      char name[256];
      sprintf(name, "Param%d", i);
      if (param->m_change == -1)
      {
        m_params[i].Unbind();
        auto checkBox = new GameMenuCheckBox();
        checkBox->SetShortProperties(param->GetStringId(), xPos, yPos += buttonH + gap, buttonW, buttonH,
                                     LANGUAGEPHRASE(param->GetStringId()));
        checkBox->RegisterNetworkInt(&m_params[i]);
        checkBox->m_fontSize = fontSize;
        RegisterButton(checkBox);
        m_buttonOrder.PutData(checkBox);
      }
      else if (param->m_change == 0)
      {
        m_params[i].Unbind();
        auto menu = new GameMenuDropDown();
        menu->SetShortProperties(param->GetStringId(), xPos, yPos += buttonH + gap, buttonW, buttonH, LANGUAGEPHRASE(param->m_name));
        for (int p = param->m_min; p <= param->m_max; ++p)
        {
          char thisOption[256];
          sprintf(thisOption, "multiwinia_option_%d_%d_%d", static_cast<int>(m_gameType), i, p);
          menu->AddOption(thisOption, p);
        }
        RegisterButton(menu);
        menu->RegisterNetworkInt(&(m_params[i]));
        menu->m_fontSize = fontSize;
        menu->m_cycleMode = true;
        m_buttonOrder.PutData(menu);
      }
      else
      {
        m_params[i].SetBounds(param->m_min, param->m_max);
        CreateMenuControl(param->GetStringId(), InputField::TypeInt, &(m_params[i]), yPos += buttonH + gap, param->m_change, nullptr, xPos,
                          buttonW, fontSize);
        UnicodeString temp;
        param->GetNameTranslated(temp);
        EclButton* b = GetButton(param->GetStringId());
        b->SetCaption(temp);
        b->m_h = buttonH;
        m_buttonOrder.PutData(b);
      }
    }

    //yPos+=buttonH + gap;

    ASSERT_TEXT(0 <= m_gameType && m_gameType < MAX_GAME_TYPES && g_app->m_gameMenu->m_maps[m_gameType].ValidIndex( m_requestedMapId ),
                "Failed to get level due to invalid game type or map id (Game Type = %d, Map ID = %d), map name (may be inaccurate) = '%s'",
                static_cast<int>(m_gameType), static_cast<int>(m_requestedMapId), g_app->m_requestedMap);

#ifdef INCLUDE_CRATES_ADVANCED
    auto crates = new GameMenuCheckBox();
    crates->SetShortProperties("multiwinia_param_basiccrates", xPos, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_param_basiccrates"));
    crates->m_fontSize = fontSize;
    RegisterButton(crates);
    crates->RegisterNetworkInt(&(m_basicCrates));
    m_buttonOrder.PutData(crates);
#endif

    if (g_app->m_gameMenu->m_maps[m_gameType][m_requestedMapId]->m_coop)
    {
      auto menu = new GameMenuCheckBox();
      menu->SetShortProperties("multiwinia_menu_coop", xPos, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_menu_coop"));
      RegisterButton(menu);
      menu->RegisterNetworkInt(&(m_coopMode));
      menu->m_fontSize = fontSize;
      m_buttonOrder.PutData(menu);
    }
  }

  yPos += buttonH + gap;

  if (g_app->m_server)
  {
    CreateMenuControl("multiwinia_menu_serverpassword", InputField::TypeUnicodeString, &m_serverPassword, yPos += buttonH + gap, 0.0f,
                      nullptr, xPos, buttonW, fontSize);
    auto pass = static_cast<GameMenuInputField*>(GetButton("multiwinia_menu_serverpassword"));
    pass->m_h = buttonH;
    m_buttonOrder.PutData(pass);
  }

  buttonH /= 0.65f;

  yPos = leftY + leftH - buttonH * 2;

  auto back = new BackPageButton(PageGameSetup);
  back->SetShortProperties("multiwinia_menu_back", xPos, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);

  xPos = rightX + (rightW * 0.05f);
  yPos = rightY + (rightH * 0.05f);
  buttonW = rightW * 0.9f;
  buttonH = rightH * 0.9f;

  auto details = new OptionDetailsButton();
  details->SetShortProperties("option_details", xPos, yPos, buttonW, buttonH, UnicodeString());
  details->m_fontSize = fontSmall;
  RegisterButton(details);
}

void GameMenuWindow::SetupPlayerOptionsPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontSmall;

  buttonH *= 0.65f;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_playeroptions"));
  title->m_fontSize = fontSize;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  CreateMenuControl("multiwinia_player_name", InputField::TypeUnicodeString, &m_teamName, yPos += buttonH + gap, -1, nullptr, buttonX,
                    buttonW, fontSize);
  auto name = static_cast<GameMenuInputField*>(GetButton("multiwinia_player_name"));
  name->m_h = buttonH;
  m_buttonOrder.PutData(name);

  yPos += buttonH;

  FancyColourButton* button = nullptr;
  if (m_gameType == Multiwinia::GameTypeAssault)
  {
    button = new FancyColourButton(-1);
    button->SetShortProperties("multiwinia_random", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_option_0_7_0"));
    button->m_fontSize = fontSize;
    RegisterButton(button);
    m_buttonOrder.PutData(button);

    button = new FancyColourButton(ASSAULT_ATTACKER_COLOURID);
    button->SetShortProperties("multiwinia_attackers", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_attackers"));
    button->m_fontSize = fontSize;
    RegisterButton(button);
    m_buttonOrder.PutData(button);

    button = new FancyColourButton(ASSAULT_DEFENDER_COLOURID);
    button->SetShortProperties("multiwinia_defenders", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_defenders"));
    button->m_fontSize = fontSize;
    RegisterButton(button);
    m_buttonOrder.PutData(button);
  }
  else
  {
    for (int i = 0; i < MULTIWINIA_NUM_TEAMCOLOURS; ++i)
    {
      button = new FancyColourButton(i);
      button->SetShortProperties(GetColourNameChar(i), buttonX, yPos += buttonH + gap, buttonW, buttonH, GetColourName(i));
      button->m_fontSize = fontSize;
      RegisterButton(button);
      m_buttonOrder.PutData(button);
    }
  }

  buttonH /= 0.65f;
  yPos = leftY + leftH - buttonH * 2;

  auto back = new ApplyNameButton(m_currentPage);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  back->m_fontSize = fontMed;
  RegisterButton(back);
  m_buttonOrder.PutData(back);

}

void GameMenuWindow::SetupFiltersPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontSmall;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_menu_filters"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  buttonH *= 0.65f;

  auto dropdown = new GameMenuDropDown(false);
  dropdown->SetShortProperties("multiwinia_gametype", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_gametype"));
  dropdown->AddOption("multiwinia_gametype_any", -1);
#if defined(HIDE_INVALID_GAMETYPES)
  if (g_app->IsGameModePermitted(Multiwinia::GameTypeSkirmish)) { 
#endif
  dropdown->AddOption("multiwinia_gametype_domination", Multiwinia::GameTypeSkirmish);
#if defined(HIDE_INVALID_GAMETYPES)
    }
  if (g_app->IsGameModePermitted(Multiwinia::GameTypeKingOfTheHill)) { 
#endif
  dropdown->AddOption("multiwinia_gametype_kingofthehill", Multiwinia::GameTypeKingOfTheHill);
#if defined(HIDE_INVALID_GAMETYPES)
    }
  if (g_app->IsGameModePermitted(Multiwinia::GameTypeCaptureTheStatue)) { 
#endif
  dropdown->AddOption("multiwinia_gametype_capturethestatue", Multiwinia::GameTypeCaptureTheStatue);
#if defined(HIDE_INVALID_GAMETYPES)
    }
  if (g_app->IsGameModePermitted(Multiwinia::GameTypeAssault)) { 
#endif
  dropdown->AddOption("multiwinia_gametype_assault", Multiwinia::GameTypeAssault);
#if defined(HIDE_INVALID_GAMETYPES)
    }
  if (g_app->IsGameModePermitted(Multiwinia::GameTypeRocketRiot)) { 
#endif
  dropdown->AddOption("multiwinia_gametype_rocketriot", Multiwinia::GameTypeRocketRiot);
#if defined(HIDE_INVALID_GAMETYPES)
    }
  if (g_app->IsGameModePermitted(Multiwinia::GameTypeBlitzkreig)) { 
#endif
  dropdown->AddOption("multiwinia_gametype_blitzkrieg", Multiwinia::GameTypeBlitzkreig);
#if defined(HIDE_INVALID_GAMETYPES)
    }
#endif
  dropdown->RegisterInt(&m_gameTypeFilter);
  dropdown->m_fontSize = fontSize;
  RegisterButton(dropdown);
  m_buttonOrder.PutData(dropdown);

  auto checkbox = new GameMenuCheckBox();
  checkbox->SetShortProperties("check_box", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_menu_showdemogames"));
  checkbox->RegisterBool(&m_showDemosFilter);
  checkbox->m_fontSize = fontSize;
  RegisterButton(checkbox);
  m_buttonOrder.PutData(checkbox);

  checkbox = new GameMenuCheckBox();
  checkbox->SetShortProperties("check_box2", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_menu_showincompatible"));
  checkbox->RegisterBool(&m_showIncompatibleGamesFilter);
  checkbox->m_fontSize = fontSize;
  RegisterButton(checkbox);
  m_buttonOrder.PutData(checkbox);

  checkbox = new GameMenuCheckBox();
  checkbox->SetShortProperties("check_box3", buttonX, yPos += buttonH + gap, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_showfull"));
  checkbox->RegisterBool(&m_showFullGamesFilter);
  checkbox->m_fontSize = fontSize;
  RegisterButton(checkbox);
  m_buttonOrder.PutData(checkbox);

  checkbox = new GameMenuCheckBox();
  checkbox->SetShortProperties("check_box5", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                               LANGUAGEPHRASE("multiwinia_menu_showpasswords"));
  checkbox->RegisterBool(&m_showPasswordedFilter);
  checkbox->m_fontSize = fontSize;
  RegisterButton(checkbox);
  m_buttonOrder.PutData(checkbox);

  buttonH /= 0.65f;
  yPos = leftY + leftH - (buttonH * 2);

  BackPageButton* back = new SaveFiltersButton(m_currentPage);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);
}

void GameMenuWindow::SetupUpdatePage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontMed;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_update_available"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  auto updater = new RunAutoPatcherButton();
  // FIXME: localize
  updater->SetShortProperties("multiwinia_menu_autopatch", buttonX, yPos += buttonH + gap, buttonW, buttonH, "Update");

  updater->m_fontSize = fontSize;
  RegisterButton(updater);
  m_buttonOrder.PutData(updater);

  yPos = leftY + leftH - (buttonH * 2);

  auto back = new BackPageButton(m_currentPage);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);
}

static void ReadMultiwiniaBetaTesters(UnicodeString& _betaTesters)
{
  auto newLine = UnicodeString(" \n");
  TextReader* reader = g_app->m_resource->GetTextReader("multiwinia_betatesters.txt");

  while (reader && reader->ReadLine())
  {
    UnicodeString line = reader->GetRestOfUnicodeLine();
    _betaTesters = _betaTesters + line + newLine;
  }
  delete reader;
}

static void ReadCredits(UnicodeString& _credits)
{
  UnicodeString betaTestersMacro = "*BETATESTERS";
  UnicodeString betaTesters;
  ReadMultiwiniaBetaTesters(betaTesters);

  auto newLine = UnicodeString(" \n");

  for (int counter = 1; ; counter++)
  {
    char key[64];
    sprintf(key, "multiwinia_credits_%d", counter);

    if (!ISLANGUAGEPHRASE(key))
      break;

    const UnicodeString& line = LANGUAGEPHRASE(key);

    if (line.StrCmp(betaTestersMacro) == 0)
      _credits = _credits + betaTesters + newLine;
    else
      _credits = _credits + line + newLine;
  }
}

void GameMenuWindow::SetupCreditsPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontMed;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("dialog_credits"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 1.3f;

  float textH = leftH - buttonH * 1.5f;
  textH -= buttonH * 1.3f;
  textH -= buttonH + gap * 2.0f;

  auto text = new ScrollingTextButton();
  text->SetShortProperties("credits_box", buttonX, yPos, buttonW, textH, UnicodeString());
  text->m_fontSize = fontSmall;
  RegisterButton(text);

  UnicodeString credits;
  ReadCredits(credits);

  text->m_string = credits;

  yPos = leftY + leftH - (buttonH * 2);

  auto back = new BackPageButton(m_currentPage);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);
}

void GameMenuWindow::SetupErrorPage()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontMed;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("error"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 2;

  auto text = new TextButton(m_errorMessage);
  text->SetShortProperties("error_text", buttonX, yPos, buttonW, buttonH, UnicodeString());
  text->m_fontSize = fontSmall;
  text->m_centered = true;
  RegisterButton(text);

  yPos = leftY + leftH - (buttonH * 2);

  auto back = new BackPageButton(m_errorBackPage);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);

  m_errorBackPage = -1;
}

void GameMenuWindow::SetupPageEnterPassword()
{
  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float buttonX = leftX + (leftW - buttonW) / 2.0f;
  float yPos = leftY;
  float fontSize = fontMed;

  yPos += buttonH * 0.5f;
  auto title = new GameMenuTitleButton();
  title->SetShortProperties("title", leftX + 10, leftY + 10, leftW - 20, buttonH * 1.5f, LANGUAGEPHRASE("multiwinia_enterpassword"));
  title->m_fontSize = fontMed;
  RegisterButton(title);
  yPos += buttonH * 2;

  auto tb = new TextButton("multiwinia_passwordrequired");
  tb->SetShortProperties("passrequired_text", buttonX, yPos, buttonW, buttonH, UnicodeString());
  tb->m_fontSize = fontSmall;
  tb->m_centered = true;
  tb->m_shadow = true;
  RegisterButton(tb);

  buttonH *= 0.65f;
  yPos += buttonH;

  CreateMenuControl("multiwinia_enterpassword", InputField::TypeUnicodeString, &m_serverPassword, yPos += buttonH + gap, 0.0, nullptr,
                    buttonX, buttonW, fontSmall);
  auto pass = static_cast<GameMenuInputField*>(GetButton("multiwinia_enterpassword"));
  pass->m_h = buttonH;
  pass->m_renderStyle2 = true;
  m_buttonOrder.PutData(pass);
  BeginTextEdit("multiwinia_enterpassword");

  buttonH /= 0.65f;

  yPos = leftY + leftH - (buttonH * 3);

  auto connect = new ServerConnectButton();
  connect->SetShortProperties("multiwinia_menu_connect", buttonX, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_connect"));
  RegisterButton(connect);
  connect->m_fontSize = fontMed;
  m_buttonOrder.PutData(connect);

  auto back = new BackPageButton(PageJoinGame);
  back->SetShortProperties("multiwinia_menu_back", buttonX, yPos += buttonH + gap, buttonW, buttonH,
                           LANGUAGEPHRASE("multiwinia_menu_back"));
  RegisterButton(back);
  back->m_fontSize = fontMed;
  m_buttonOrder.PutData(back);
}

GameMenuInputField* GameMenuWindow::CreateMenuControl(const char* name, int dataType, NetworkValue* value, int y, float change,
                                                      DarwiniaButton* callback, int x, int w, float fontSize)
{
  if (x == -1)
    x = 10;
  if (w == -1)
    w = m_w - x * 2;

  auto input = new GameMenuInputField();
  input->SetShortProperties(name, x, y, w, 30, LANGUAGEPHRASE(name));
  input->SetCallback(callback);
  input->m_fontSize = fontSize;
  input->RegisterNetworkValue(dataType, value);
  RegisterButton(input);

  if (dataType != InputField::TypeString && dataType != InputField::TypeUnicodeString)
  {
    float texY = y + fontSize / 2;
    char nameLeft[64];
    sprintf(nameLeft, "%s left", name);
    auto left = new GameMenuInputScroller();
    left->SetProperties(nameLeft, input->m_x + input->m_w - fontSize * 4.0f, texY, fontSize, fontSize, UnicodeString("<"),
                        UnicodeString("Value left"));
    left->m_inputField = input;
    left->m_change = -change;
    left->m_fontSize = fontSize;
    RegisterButton(left);

    char nameRight[64];
    sprintf(nameRight, "%s right", name);
    auto right = new GameMenuInputScroller();
    right->SetProperties(nameRight, input->m_x + input->m_w - fontSize, texY, fontSize, fontSize, UnicodeString(">"),
                         UnicodeString("Value right"));
    right->m_inputField = input;
    right->m_change = change;
    right->m_fontSize = fontSize;
    RegisterButton(right);
  }

  return input;
}

void GameMenuWindow::GetDefaultPositions(int* _x, int* _y, int* _gap)
{
  float w = g_app->m_renderer->ScreenW();
  float h = g_app->m_renderer->ScreenH();

  *_x = static_cast<float>((w / 1152) * 200.0f);
  switch (m_newPage)
  {
  case PageMain:
  case PageDarwinia:
    *_y = static_cast<float>((h / 864.0f) * 200.0f);
    *_gap = *_y;
    break;
  case PageNewOrJoin:
  case PageJoinGame:
  case PageGameConnecting:
  case PageGameSelect:
    *_y = static_cast<float>((h / 864.0f) * 175.0f);
    *_gap = *_y / 1.5f;
    break;
  case PageGameSetup:
  case PageResearch:
    *_y = static_cast<float>((h / 864.0f) * 70.0f);
    *_gap = (h / 864) * 60;
    break;
  }

  *_x = min(*_x, 200);
}
