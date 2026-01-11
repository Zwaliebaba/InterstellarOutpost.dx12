#ifndef __ACHIEVEMENTBUTTON__
#define __ACHIEVEMENTBUTTON__

#include "scrollbar.h"
#include "game_menu.h"
#include "achievement_tracker.h"

class AchievementButton : public GameMenuButton
{
  bool m_loadedImage;
  bool m_attemptedLoad;
  double m_startTime;
  unsigned int m_imageWidth;
  unsigned int m_imageHeight;
  LList<UnicodeString*>* m_wrapped;

  public:
    float m_yOffset;
    bool m_reset;

    AchievementButton(int _achievementNumber)
      : GameMenuButton(""),
        m_loadedImage(false),
        m_attemptedLoad(false),
        m_startTime(-1),
        m_imageWidth(0),
        m_imageHeight(0),
        m_wrapped(nullptr),
        m_yOffset(0),
        m_reset(true)
    {
      sprintf(m_name, "achievement%d", _achievementNumber);
      m_caption = UnicodeString();
    }

    ~AchievementButton() override
    {
      if (m_wrapped)
      {
        m_wrapped->EmptyAndDelete();
        delete m_wrapped;
      }
    }

    bool HasLoadedImage() { return m_loadedImage || m_attemptedLoad; }

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      if (m_reset)
      {
        if (!g_app->m_renderer->IsMenuTransitionComplete())
          return;

        m_startTime = GetHighResTime();
        m_reset = false;
      }

      auto gmw = static_cast<GameMenuWindow*>(m_parent);
      int offset = 0;
      if (gmw->m_scrollBar)
        offset = gmw->m_scrollBar->m_currentValue;

      float loadPos = (GetHighResTime() - m_startTime) * 750;
      loadPos = min(loadPos, m_yOffset);
      int position = realY - (offset * 4) + loadPos;

      if (position < gmw->m_scrollBar->m_y - (m_imageHeight / 10) - m_h || position > gmw->m_scrollBar->m_y - (m_imageHeight / 10) + gmw->
        m_scrollBar->m_h)
        return;

      realY = position;

      float leftX, leftY, leftW, leftH;
      float titleX, titleY, titleW, titleH;
      GetPosition_LeftBox(leftX, leftY, leftW, leftH);
      GetPosition_TitleBar(titleX, titleY, titleW, titleH);

      g_app->m_renderer->Clip(leftX, gmw->m_scrollBar->m_y - (m_imageHeight / 10), m_w, gmw->m_scrollBar->m_h);

      // Background colour
      glColor4f(0.4f, 0.4f, 0.4f, 0.5f);
      glBegin(GL_QUADS);
      glVertex2f(realX - m_imageWidth / 10, realY - m_imageHeight / 10);
      glVertex2f(realX + m_w, realY - m_imageHeight / 10);
      glVertex2f(realX + m_w, realY + m_imageHeight * 1.2f);
      glVertex2f(realX - m_imageWidth / 10, realY + m_imageHeight * 1.2f);
      glEnd();
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

      m_disabled = true;
      m_inactive = true;

      g_app->m_renderer->EndClip();
    }

    void MouseUp() override {}

    char* MakeNumberHumanReadable(int _num, char* _out)
    {
      char temp[256];
      if (static_cast<int>(_num / 1000) > 0)
      {
        sprintf(temp, "%s,%.3d", MakeNumberHumanReadable(_num / 1000, _out), _num - (static_cast<int>(_num / 1000) * 1000));
        sprintf(_out, temp);
      }
      else
        sprintf(_out, "%d", _num);
      return _out;
    }
};

#endif
