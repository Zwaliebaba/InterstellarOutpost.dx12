#ifndef __ADVANCEDOPTIONSMODEBUTTON__
#define __ADVANCEDOPTIONSMODEBUTTON__

class AdvancedOptionsModeButton : public GameMenuButton
{
  public:
    AdvancedOptionsModeButton()
      : GameMenuButton("AdvancedOptions") {}

    void MouseUp() override
    {
      GameMenuButton::MouseUp();

      static_cast<GameMenuWindow*>(m_parent)->m_newPage = GameMenuWindow::PageAdvancedOptions;
    }
};

#endif
