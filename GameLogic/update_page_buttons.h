#ifndef _included_update_page_buttons_h
#define _included_update_page_buttons_h

class UpdatePageButton : public GameMenuButton
{
  public:
    UpdatePageButton()
      : GameMenuButton(UnicodeString()) {}

    void MouseUp() override
    {
      auto window = static_cast<GameMenuWindow*>(m_parent);
      window->m_newPage = GameMenuWindow::PageUpdateAvailable;

      GameMenuButton::MouseUp();
    }
};

class RunAutoPatcherButton : public GameMenuButton
{
  public:
    RunAutoPatcherButton()
      : GameMenuButton(UnicodeString()) {}

#if defined(TARGET_MSVC)
    void MouseUp() override
    {
      STARTUPINFOA si;
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);
      PROCESS_INFORMATION pi;
      ZeroMemory(&pi, sizeof(pi));
      CreateProcessA("autopatch.exe", nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      exit(0);
    }
#endif
};

#endif
