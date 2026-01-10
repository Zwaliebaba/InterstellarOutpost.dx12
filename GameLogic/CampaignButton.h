#ifndef __CAMPAIGNBUTTON__
#define __CAMPAIGNBUTTON__

#include "main.h"

class CampaignButton : public GameMenuButton
{
  public:
    CampaignButton(char* _iconName)
      : GameMenuButton(_iconName) {}

    static void LoadCampaign()
    {
      if (g_app->m_server)
        g_app->ShutdownCurrentGame();

      RequestBootloaderSequence();
      strcpy(g_app->m_gameDataFile, "game.txt");
      g_app->m_gameMode = App::GameModeCampaign;
      g_app->LoadProfile();

      g_app->m_atMainMenu = false;
    }

    void MouseUp() override
    {
      GameMenuButton::MouseUp();

      g_app->m_loadingLocation = true;
      g_loadingScreen->m_workQueue->Add(&LoadCampaign);
      g_loadingScreen->Render();
    }

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      GameMenuButton::Render(realX, realY, highlighted, clicked);
      auto parent = static_cast<GameMenuWindow*>(m_parent);
      if (parent->m_buttonOrder[parent->m_currentButton] == this)
        highlighted = true;
      if (highlighted)
      {
        auto parent = static_cast<GameMenuWindow*>(m_parent);
        parent->m_highlightedGameType = GAMETYPE_CAMPAIGN;
      }
        m_inactive = false;
    }
};

#endif
