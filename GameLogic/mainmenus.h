#ifndef _included_mainmenus_h
#define _included_mainmenus_h

#include "language_table.h"
#include "darwinia_window.h"
#include "game_menu.h"
#include "app.h"
#include "renderer.h"
#include "soundsystem.h"
#include "resource.h"
#include "text_renderer.h"

class GameOptionsWindow : public DarwiniaWindow
{
  public:
    bool m_renderRightBox;
    bool m_renderWholeScreen;

    UnicodeString m_errorMessage;
    bool m_showingErrorDialogue;
    bool m_dialogueSuccessMessage;

    GameOptionsWindow(const char* _name);
    void Create() override;

    void Update() override;
    void Render(bool _hasFocus) override;
    GameMenuInputField* CreateMenuControl(const char* name, int dataType, NetworkValue* value, int y, float change,
                                          DarwiniaButton* callback, int x, int w, float fontSize);

    void CreateErrorDialogue(UnicodeString _error, bool _success = false);
    void RenderErrorDialogue();
};

class MainMenuWindow : public GameOptionsWindow
{
  public:
    MainMenuWindow();

    void Create() override;
    void Render(bool _hasFocus) override;
};

class OptionsMenuWindow : public GameOptionsWindow
{
  public:
    OptionsMenuWindow();

    void Create() override;
};

class LocationWindow : public GameOptionsWindow
{
  public:
    LocationWindow();
    ~LocationWindow() override;

    void Create() override;
};

class ResetLocationWindow : public GameOptionsWindow
{
  public:
    ResetLocationWindow();

    void Create() override;
    void Render(bool _hasFocus) override;
};

class AboutDarwiniaWindow : public DarwiniaWindow
{
  public:
    AboutDarwiniaWindow();

    void Create() override;
    void Render(bool _hasFocus) override;
};

class ConfirmExitWindow : public GameOptionsWindow
{
  public:
    ConfirmExitWindow();
    void Create() override;
};

class ConfirmResetWindow : public GameOptionsWindow
{
  public:
    ConfirmResetWindow();
    void Create() override;
};

class MenuCloseButton : public GameMenuButton
{
  public:
    MenuCloseButton(char* _name)
      : GameMenuButton(_name) {}

    void MouseUp() override
    {
      g_app->m_soundSystem->TriggerOtherEvent(nullptr, "MenuCancel", SoundSourceBlueprint::TypeMultiwiniaInterface);
      g_app->m_renderer->InitialiseMenuTransition(1.0f, -1);
      DebugTrace("Removing window %s\n", m_parent->m_name);
      EclRemoveWindow(m_parent->m_name);
      g_app->m_doMenuTransition = true;
    }

    void Render(int realX, int realY, bool highlighted, bool clicked) override
    {
      GameMenuButton::Render(realX, realY, highlighted, clicked);

      if (g_inputManager->getInputMode() == INPUT_MODE_GAMEPAD)
      {
        float iconSize = m_fontSize;
        float iconAlpha = 1.0f;
        int xPos = realX + g_gameFont.GetTextWidth(m_caption.Length(), m_fontSize) + 60;
        int yPos = realY + (m_h / 2.0f);

        if (m_centered)
          xPos += m_w / 2.0f;

        auto iconCentre = Vector2(xPos, yPos);

        // Render the icon

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_app->m_resource->GetTexture("icons\\button_b.bmp"));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 1.0f, 1.0f, iconAlpha);

        float x = iconSize / 2;

        glBegin(GL_QUADS);
        glTexCoord2i(0, 1);
        glVertex2f(iconCentre.x - x, iconCentre.y - iconSize / 2);
        glTexCoord2i(1, 1);
        glVertex2f(iconCentre.x + x, iconCentre.y - iconSize / 2);
        glTexCoord2i(1, 0);
        glVertex2f(iconCentre.x + x, iconCentre.y + iconSize / 2);
        glTexCoord2i(0, 0);
        glVertex2f(iconCentre.x - x, iconCentre.y + iconSize / 2);
        glEnd();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
      }
    }
};

class MenuGameExitButton : public GameMenuButton
{
  public:
    MenuGameExitButton()
      : GameMenuButton(LANGUAGEPHRASE("dialog_leavedarwinia")) {}

    void MouseUp() override
    {
      g_app->m_atMainMenu = true;
      g_app->m_renderer->StartFadeOut();
    }
};

class HelpAndOptionsButton : public GameMenuButton
{
  public:
    HelpAndOptionsButton();
    void MouseUp() override;
};

class AchievementsButton : public GameMenuButton
{
  public:
    AchievementsButton()
      : GameMenuButton("multiwinia_menu_achievements") {}
};

class ConfirmResetButton : public GameMenuButton
{
  public:
    ConfirmResetButton();

    void MouseUp() override;
};

#endif
