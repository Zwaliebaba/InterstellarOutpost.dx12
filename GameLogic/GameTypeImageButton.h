#ifndef __GAMETYPEIMAGEBUTTON__
#define __GAMETYPEIMAGEBUTTON__

class GameTypeImageButton : public DarwiniaButton
{
  public:
    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      auto parent = static_cast<GameMenuWindow*>(m_parent);
      int gameType = parent->m_gameType;
      if (gameType == -1 || parent->m_currentPage == GameMenuWindow::PageGameSelect)
        gameType = parent->m_highlightedGameType;

      char filename[256];
      if (Multiwinia::s_gameBlueprints.ValidIndex(gameType))
      {
        sprintf(filename, "mwhelp/mode_%s." TEXTURE_EXTENSION, Multiwinia::s_gameBlueprints[gameType]->m_name);
        strlwr(filename);
      }
      else if (gameType == GAMETYPE_PROLOGUE)
      {
        sprintf(filename, "mwhelp/mode_prologue." TEXTURE_EXTENSION);
        strlwr(filename);
      }
      else if (gameType == GAMETYPE_CAMPAIGN)
      {
        sprintf(filename, "mwhelp/mode_campaign." TEXTURE_EXTENSION);
        strlwr(filename);
      }
      else
      {
        DebugTrace("GameTypeImageButton::Render gameType == -1\n");
        return;
      }

      if (g_app->m_resource->DoesTextureExist(filename))
      {
        int texId = g_app->m_resource->GetTexture(filename, false, false);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        float fullW = g_app->m_renderer->ScreenW() * 1.5f;
        float fullH = fullW * 200.0f / 300.0f;
        float fullX = -fullW * 0.25f;
        float fullY = 0;

        glColor4f(1.0f, 1.0f, 1.0f, 0.1f);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 1);
        glVertex2f(realX, realY);
        glTexCoord2i(1, 1);
        glVertex2f(realX + m_w, realY);
        glTexCoord2i(1, 0);
        glVertex2f(realX + m_w, realY + m_h);
        glTexCoord2i(0, 0);
        glVertex2f(realX, realY + m_h);
        glEnd();

        glDisable(GL_TEXTURE_2D);
      }

      glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

      glBegin(GL_LINE_LOOP);
      glVertex2f(realX, realY);
      glVertex2f(realX + m_w, realY);
      glVertex2f(realX + m_w, realY + m_h);
      glVertex2f(realX, realY + m_h);
      glEnd();

    }
};

#endif
