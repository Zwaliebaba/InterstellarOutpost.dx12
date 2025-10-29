#include "pch.h"
#include "debug_utils.h"
#include "text_renderer.h"
#include "preferences.h"
#include "window_manager.h"
#include "language_table.h"
#include "interface/debugmenu.h"
#include "interface/gesture_window.h"
#include "interface/grabber_window.h"
#include "interface/network_window.h"
#include "interface/prefs_screen_window.h"
#include "interface/prefs_graphics_window.h"
#include "interface/prefs_sound_window.h"
#include "interface/profilewindow.h"
#include "interface/pokey_window.h"
#include "interface/soundeditor_window.h"
#include "interface/sound_profile_window.h"
#include "interface/soundstats_window.h"
#include "interface/cheat_window.h"
#include "interface/reallyquit_window.h"
#include "app.h"
#include "camera.h"
#include "renderer.h"

// ****************************************************************************
// Menu Buttons
// ****************************************************************************

#ifdef PROFILER_ENABLED
class ProfileButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::ProfileButton(); }
};
#endif // PROFILER_ENABLED


class NetworkButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::NetworkButton(); }
};


#ifdef LOCATION_EDITOR
class EditorButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::EditorButton(); }
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
  void MouseUp() override { if (!EclGetWindow(LANGUAGEPHRASE("dialog_screenoptions"))) { EclRegisterWindow(new PrefsScreenWindow()); } }
};


class PrefsGfxDetailButton : public DarwiniaButton
{
public:
  void MouseUp() override { if (!EclGetWindow(LANGUAGEPHRASE("dialog_graphicsoptions"))) { EclRegisterWindow(new PrefsGraphicsWindow()); } }
};


class PrefsSoundButton : public DarwiniaButton
{
public:
  void MouseUp() override { if (!EclGetWindow(LANGUAGEPHRASE("dialog_soundoptions"))) { EclRegisterWindow(new PrefsSoundWindow()); } };
};


#ifdef CHEATMENU_ENABLED
class CheatButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::CheatButton(); }
};
#endif


#ifdef SOUND_EDITOR
class SoundStatsButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::SoundStatsButton(); }
};
#endif // SOUND_EDITOR


#ifdef SOUND_EDITOR
class SoundProfileButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::SoundProfileButton(); }
};
#endif // SOUND_EDITOR


#ifdef SOUND_EDITOR
class SoundEditorButton : public DarwiniaButton
{
public:
  void MouseUp() override { DebugKeyBindings::SoundEditorButton(); }
};
#endif // SOUND_EDITOR


// ****************************************************************************
// Class DebugMenu
// ****************************************************************************

DebugMenu::DebugMenu(char *name)
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

  DarwiniaButton *button;

#ifdef PROFILER_ENABLED
  button = new ProfileButton();
  button->SetShortProperties("Profile (F6)", 10, y += pitch, m_w - 20);
  RegisterButton(button);
#endif // PROFILER_ENABLED

  button = new NetworkButton();
  button->SetShortProperties("Network Stats", 10, y += pitch, m_w - 20);
  RegisterButton(button);

  button = new FPSButton();
  button->SetShortProperties("Display FPS (F5)", 10, y += pitch, m_w - 20);
  RegisterButton(button);

  y += pitch / 2.0f;

  y += pitch / 2.0f;

  button = new DebugCameraButton();
  button->SetShortProperties("Dbg Cam (F2)", 10, y += pitch, m_w - 20);
  RegisterButton(button);

  y += pitch / 2.0f;

  bool modsEnabled = g_prefsManager->GetInt("ModSystemEnabled", 0) != 0;

#ifdef LOCATION_EDITOR
  if (modsEnabled)
  {
    button = new EditorButton();
    button->SetShortProperties("Toggle Editor (F3)", 10, y += pitch, m_w - 20);
    RegisterButton(button);
  }
#endif // LOCATION_EDITOR

#ifdef CHEATMENU_ENABLED
  button = new CheatButton();
  button->SetShortProperties("Cheat Menu (F4)", 10, y += pitch, m_w - 20);
  RegisterButton(button);
#endif
}


void DebugMenu::Render(bool hasFocus)
{
  Advance();

  DarwiniaWindow::Render(hasFocus);

  EclButton *camDbgButton = GetButton("Dbg Cam (F2)");
  DarwiniaDebugAssert(camDbgButton);
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
  char *debugMenuWindowName = LANGUAGEPHRASE("dialog_toolsmenu");
  if (EclGetWindow(debugMenuWindowName)) EclRemoveWindow(debugMenuWindowName);
  else EclRegisterWindow(new ::DebugMenu(debugMenuWindowName));
}

#ifdef PROFILER_ENABLED
void DebugKeyBindings::ProfileButton()
{
  if (EclGetWindow("Profiler")) { EclRemoveWindow("Profiler"); }
  else
  {
    auto pw = new ProfileWindow("Profiler");
    pw->m_w = 570;
    pw->m_h = 450;
    pw->m_x = g_app->m_renderer->ScreenW() - pw->m_w - 20;
    pw->m_y = 30;
    EclRegisterWindow(pw);
  }
}
#endif


void DebugKeyBindings::NetworkButton()
{
  if (!EclGetWindow("Network Stats"))
  {
    auto nw = new NetworkWindow("Network Stats");
    nw->m_w = 200;
    nw->m_h = 200;
    nw->m_x = 10;
    nw->m_y = g_app->m_renderer->ScreenH() - nw->m_h;
    EclRegisterWindow(nw);
  }
}


#ifdef LOCATION_EDITOR
void DebugKeyBindings::EditorButton() { g_app->m_requestToggleEditing = true; }
#endif // LOCATION_EDITOR


#ifdef AVI_GENERATOR
void DebugKeyBindings::GrabberButton()
{
  if (!EclGetWindow("Grabber"))
  {
    auto gw = new GrabberWindow("Grabber");
    gw->m_w = 200;
    gw->m_h = 50;
    gw->m_x = 10;
    gw->m_y = g_app->m_renderer->ScreenH() - gw->m_h;
    EclRegisterWindow(gw);
  }
}
#endif // AVI_GENERATOR

void DebugKeyBindings::DebugCameraButton() { g_app->m_camera->SetNextDebugMode(); }

void DebugKeyBindings::FPSButton() { g_app->m_renderer->m_displayFPS = !g_app->m_renderer->m_displayFPS; }


#ifdef SOUND_EDITOR
void DebugKeyBindings::PokeyButton()
{
  if (!EclGetWindow("Pokey Playground"))
  {
    auto pokeyWin = new PokeyWindow("Pokey Playground");
    pokeyWin->m_w = 400;
    pokeyWin->m_h = 480;
    pokeyWin->m_x = 10;
    pokeyWin->m_y = 40;
    EclRegisterWindow(pokeyWin);
  }
}
#endif // SOUND_EDITOR


#ifdef GESTURE_EDITOR
void DebugKeyBindings::GestureButton()
{
  if (!EclGetWindow("Gesture Editor"))
  {
    auto gesture = new GestureWindow("Gesture Editor");
    gesture->m_w = 660;
    gesture->m_h = 660;
    gesture->m_x = 30;
    gesture->m_y = 30;
    EclRegisterWindow(gesture);
  }
}
#endif // GESTURE_EDITOR


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

#ifdef SOUND_EDITOR
void DebugKeyBindings::SoundStatsButton()
{
  if (EclGetWindow(SOUND_STATS_WINDOW_NAME)) { EclRemoveWindow(SOUND_STATS_WINDOW_NAME); }
  else
  {
    auto window = new SoundStatsWindow(SOUND_STATS_WINDOW_NAME);
    EclRegisterWindow(window);
  }
}
#endif // SOUND_EDITOR


#ifdef SOUND_EDITOR
void DebugKeyBindings::SoundProfileButton()
{
  if (EclGetWindow(SOUND_PROFILE_WINDOW_NAME)) { EclRemoveWindow(SOUND_PROFILE_WINDOW_NAME); }
  else
  {
    auto window = new SoundProfileWindow(SOUND_PROFILE_WINDOW_NAME);
    EclRegisterWindow(window);
  }
}
#endif // SOUND_EDITOR


#ifdef SOUND_EDITOR
void DebugKeyBindings::SoundEditorButton()
{
  if (EclGetWindow(SOUND_EDITOR_WINDOW_NAME)) { EclRemoveWindow(SOUND_EDITOR_WINDOW_NAME); }
  else
  {
    auto sound = new SoundEditorWindow(SOUND_EDITOR_WINDOW_NAME);
    sound->m_w = 470;
    sound->m_h = 550;
    sound->m_x = 50;
    sound->m_y = 25;
    EclRegisterWindow(sound);
  }
}
#endif // SOUND_EDITOR

void DebugKeyBindings::ReallyQuitButton()
{
  // Bring up a really quit window
  if (!EclGetWindow(REALLYQUIT_WINDOWNAME)) EclRegisterWindow(new ReallyQuitWindow());
}

void DebugKeyBindings::ToggleFullscreenButton()
{
  bool switchingToWindowed;
  SetWindowed(!g_windowManager->Windowed(), true, switchingToWindowed);
}