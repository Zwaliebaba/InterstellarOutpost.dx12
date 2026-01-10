#ifndef __PLAYGAMEBUTTON__
#define __PLAYGAMEBUTTON__

class PlayGameButton : public GameMenuButton
{
  public:
    PlayGameButton()
      : GameMenuButton("Play") {}

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      auto parent = static_cast<GameMenuWindow*>(m_parent);
      GameMenuButton::Render(realX, realY, highlighted, clicked);
    }

    void MouseUp() override
    {
      auto parent = static_cast<GameMenuWindow*>(m_parent);
      if (parent->m_singlePlayer)
      {
        if (parent->m_localSelectedLevel != parent->m_requestedMapId)
          return;

        //GameMenuButton::MouseUp();

        if (0 <= parent->m_gameType && parent->m_gameType < MAX_GAME_TYPES && g_app->m_gameMenu->m_maps[parent->m_gameType].
          ValidIndex(parent->m_requestedMapId))
        {
          MapData* mapData = g_app->m_gameMenu->m_maps[parent->m_gameType][parent->m_requestedMapId];
          mapData->CalculeMapId();

          g_app->m_soundSystem->WaitIsInitialized();

          // fill up any empty player slots with AI teams before starting the game
          for (int i = 0; i < NUM_TEAMS; ++i)
          {
            if (i < g_app->m_gameMenu->m_maps[parent->m_gameType][parent->m_requestedMapId]->m_numPlayers && g_app->m_multiwinia->m_teams[i]
              .m_teamType == TeamTypeUnused)
              g_app->m_clientToServer->RequestTeam(TeamTypeCPU, -1);
          }

          parent->UpdateGameOptions(true);
          g_app->m_clientToServer->RequestStartGame();
        }
        else
        {
          DebugTrace("Game Menu: Failed to start game due to invalid game type or map id (Game Type = %d, Map ID = %d)\n",
                     static_cast<int>(parent->m_gameType), static_cast<int>(parent->m_requestedMapId));
        }
      }
      else
        g_app->m_clientToServer->RequestToggleReady(g_app->m_globalWorld->m_myTeamId);
    }
};

#endif
