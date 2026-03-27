#pragma once

#include "GameApp.h"
#include "GameMenuWindow.h"
#include "text_renderer.h"
#include "soundsystem.h"

class MenuCloseButton : public GameMenuButton
{
  public:
    MenuCloseButton(const char* _name) : GameMenuButton(_name) {}

    void MouseUp() override
    {
      g_app->m_soundSystem->TriggerOtherEvent(nullptr, "MenuCancel", SoundSourceBlueprint::TypeMultiwiniaInterface);
      DebugTrace("Removing window {}\n", m_parent->m_name);
      EclRemoveWindow(m_parent->m_name);
    }
};
