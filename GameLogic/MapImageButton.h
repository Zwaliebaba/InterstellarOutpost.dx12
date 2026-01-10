#ifndef __MAPIMAGEBUTTON__
#define __MAPIMAGEBUTTON__

class MapImageButton : public DarwiniaButton
{
  public:
    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

      int gameType = static_cast<GameMenuWindow*>(m_parent)->m_gameType;
      int mapId = static_cast<GameMenuWindow*>(m_parent)->m_highlightedLevel;

      if (gameType == -1)
        gameType = static_cast<GameMenuWindow*>(m_parent)->m_highlightedGameType;

      if (gameType >= 0 && gameType < Multiwinia::NumGameTypes)
      {
        if (!g_app->m_gameMenu->m_maps[gameType].ValidIndex(mapId))
          mapId = static_cast<GameMenuWindow*>(m_parent)->m_requestedMapId;

        if (g_app->m_gameMenu->m_maps[gameType].ValidIndex(mapId))
        {
          MapData* md = g_app->m_gameMenu->m_maps[gameType][mapId];

          char filename[256];
          char thumbfile[256];
          strcpy(thumbfile, md->m_fileName);
          thumbfile[strlen(thumbfile) - 4] = '\0';
          strcat(thumbfile, "." TEXTURE_EXTENSION);
          sprintf(filename, "thumbnails/%s", thumbfile);
          strlwr(filename);
          bool exists = g_app->m_resource->DoesTextureExist(filename);
          if (!exists)
            sprintf(filename, "thumbnails/default." TEXTURE_EXTENSION);

          if (exists)
          {
            int texId = g_app->m_resource->GetTexture(filename, false, false);
            if (texId == -1)
            {
              sprintf(filename, "thumbnails/default." TEXTURE_EXTENSION);
              texId = g_app->m_resource->GetTexture(filename, false, false);
            }

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
        }
      }

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glBegin(GL_LINE_LOOP);
      glVertex2f(realX, realY);
      glVertex2f(realX + m_w, realY);
      glVertex2f(realX + m_w, realY + m_h);
      glVertex2f(realX, realY + m_h);
      glEnd();

    }
};

#endif
