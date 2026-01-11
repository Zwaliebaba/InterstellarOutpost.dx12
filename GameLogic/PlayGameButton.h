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
      g_app->m_clientToServer->RequestToggleReady(g_app->m_globalWorld->m_myTeamId);
    }
};

#endif
