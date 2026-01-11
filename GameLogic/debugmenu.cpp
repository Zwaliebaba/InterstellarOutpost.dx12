#include "pch.h"
#include "text_renderer.h"
#include "preferences.h"
#include "window_manager.h"
#include "language_table.h"
#include "debugmenu.h"
#include "network_window.h"
#include "prefs_screen_window.h"
#include "prefs_graphics_window.h"
#include "prefs_sound_window.h"
#include "profilewindow.h"
#include "cheat_window.h"
#include "reallyquit_window.h"
#include "app.h"
#include "camera.h"
#include "renderer.h"

// ****************************************************************************
// Menu Buttons
// ****************************************************************************

class ProfileButton : public DarwiniaButton
{
  public:
    void MouseUp() override { DebugKeyBindings::ProfileButton(); }
};

class NetworkButton : public DarwiniaButton
{
  public:
    void MouseUp() override { DebugKeyBindings::NetworkButton(); }
};

#ifdef LOCATION_EDITOR
class EditorButton : public DarwiniaButton
{
  public:
    void MouseUp() { DebugKeyBindings::EditorButton(); }
};
#endif // LOCATION_EDITOR

class DebugCameraButton : public DarwiniaButton
{
  public:
    void MouseUp() override { DebugKeyBindings::DebugCameraButton(); }
};

class FPSButton : public DarwiniaButton
{
  public:
    void MouseUp() override { DebugKeyBindings::FPSButton(); }
};

class PrefsScreenButton : public DarwiniaButton
{
  public:
    void MouseUp() override
    {
      if (!EclGetWindow("dialog_screenoptions"))
        EclRegisterWindow(new PrefsScreenWindow());
    }
};

class PrefsGfxDetailButton : public DarwiniaButton
{
  public:
    void MouseUp() override
    {
      if (!EclGetWindow("dialog_graphicsoptions"))
        EclRegisterWindow(new PrefsGraphicsWindow());
    }
};

class PrefsSoundButton : public DarwiniaButton
{
  public:
    void MouseUp() override
    {
      if (!EclGetWindow("dialog_soundoptions"))
        EclRegisterWindow(new PrefsSoundWindow());
    };
};

#ifdef CHEATMENU_ENABLED
class CheatButton : public DarwiniaButton
{
  public:
    void MouseUp() override { DebugKeyBindings::CheatButton(); }
};
#endif

// ****************************************************************************
// Class DebugKeyBindings
// ****************************************************************************

void DebugKeyBindings::ProfileButton()
{
  if (EclGetWindow("Profiler"))
    EclRemoveWindow("Profiler");
  else
  {
    auto pw = new ProfileWindow("Profiler");
    pw->m_w = 570;
    pw->m_h = 600;
    pw->m_x = g_app->m_renderer->ScreenW() - pw->m_w - 20;
    pw->m_y = 30;
    EclRegisterWindow(pw);
  }
}

void DebugKeyBindings::NetworkButton()
{
  if (!EclGetWindow("Network Stats"))
  {
    auto nw = new NetworkWindow("Network Stats");
    EclRegisterWindow(nw);
  }
}

#ifdef LOCATION_EDITOR
void DebugKeyBindings::EditorButton() { g_app->m_requestToggleEditing = true; }
#endif // LOCATION_EDITOR

void DebugKeyBindings::DebugCameraButton() { g_app->m_camera->SetNextDebugMode(); }

void DebugKeyBindings::FPSButton() { g_app->m_renderer->m_displayFPS = !g_app->m_renderer->m_displayFPS; }

#ifdef CHEATMENU_ENABLED
void DebugKeyBindings::CheatButton()
{
  if (!EclGetWindow("Cheat Window"))
  {
    auto window = new CheatWindow("Cheat Window");
    window->m_w = 200;
    window->m_h = 200;
    window->m_x = 250;
    window->m_y = 50;
    EclRegisterWindow(window);
  }
}
#endif
