#ifndef __NEWORJOINBUTTON__
#define __NEWORJOINBUTTON__

class NewOrJoinButton : public GameMenuButton
{
  public:
    NewOrJoinButton(const char* _iconName)
      : GameMenuButton(_iconName) {}

    void MouseUp() override
    {
      GameMenuButton::MouseUp();

      static_cast<GameMenuWindow*>(m_parent)->m_newPage = GameMenuWindow::PageNewOrJoin;

      LockController();
    }
};

#endif
