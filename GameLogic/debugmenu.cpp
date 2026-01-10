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
// Class DebugMenu
// ****************************************************************************

DebugMenu::DebugMenu(const char* name)
  : DarwiniaWindow(name)
{
  m_x = 10;
  m_y = 20;
  m_w = 170;
  m_h = 75;
}

void DebugMenu::Advance() {}

void DebugMenu::Create()
{
  DarwiniaWindow::Create();

  int pitch = 18;
  int y = 5;

  DarwiniaButton* button;

  button = new ProfileButton();
  button->SetShortProperties("Profile (F6)", 10, y += pitch, m_w - 20, -1, UnicodeString("Profile (F6)"));
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  button = new NetworkButton();
  button->SetShortProperties("Network Stats", 10, y += pitch, m_w - 20, -1, UnicodeString("Network Stats"));
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  button = new FPSButton();
  button->SetShortProperties("Display FPS (F5)", 10, y += pitch, m_w - 20, -1, UnicodeString("Display FPS (F5)"));
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  y += pitch / 2.0;

  y += pitch / 2.0;

  button = new DebugCameraButton();
  button->SetShortProperties("Dbg Cam (F2)", 10, y += pitch, m_w - 20, -1, UnicodeString("Dbg Cam (F2)"));
  RegisterButton(button);
  m_buttonOrder.PutData(button);

  y += pitch / 2.0;

  bool modsEnabled = false;

#ifdef LOCATION_EDITOR
  if (modsEnabled)
  {
    button = new EditorButton();
    button->SetShortProperties("Toggle Editor (F3)", 10, y += pitch, m_w - 20, -1, UnicodeString("Toggle Editor (F3)"));
    RegisterButton(button);
    m_buttonOrder.PutData(button);
  }
#endif // LOCATION_EDITOR

#ifdef CHEATMENU_ENABLED
  button = new CheatButton();
  button->SetShortProperties("Cheat Menu (F4)", 10, y += pitch, m_w - 20, -1, UnicodeString("Cheat Menu (F4)"));
  RegisterButton(button);
#endif
}

void DebugMenu::Render(bool hasFocus)
{
  Advance();

  DarwiniaWindow::Render(hasFocus);

  EclButton* camDbgButton = GetButton("Dbg Cam (F2)");
  DEBUG_ASSERT(camDbgButton);
  int y = m_y + camDbgButton->m_y + 11;

  switch (g_app->m_camera->GetDebugMode())
  {
  case Camera::DebugModeAlways:
    g_editorFont.DrawText2D(m_x + m_w - 47, y, 10, "Always");
    break;
  case Camera::DebugModeAuto:
    g_editorFont.DrawText2D(m_x + m_w - 47, y, 10, "Auto");
    break;
  case Camera::DebugModeNever:
    g_editorFont.DrawText2D(m_x + m_w - 47, y, 10, "Never");
    break;
  }
}

// ****************************************************************************
// Class DebugKeyBindings
// ****************************************************************************

void DebugKeyBindings::DebugMenu()
{
  char* debugMenuWindowName = "dialog_toolsmenu";
  if (EclGetWindow(debugMenuWindowName))
    EclRemoveWindow(debugMenuWindowName);
  else
    EclRegisterWindow(new ::DebugMenu(debugMenuWindowName));
}

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

void DebugKeyBindings::ReallyQuitButton()
{
  // Bring up a really quit window
  if (!EclGetWindow(REALLYQUIT_WINDOWNAME))
    EclRegisterWindow(new ReallyQuitWindow());
}

void DebugKeyBindings::ToggleFullscreenButton()
{
  bool switchingToWindowed;
  SetWindowed(!g_windowManager->Windowed(), true, switchingToWindowed);
}
