#include "pch.h"
#include "filesys_utils.h"
#include "language_table.h"
#include "preferences.h"
#include "userprofile_window.h"
#include "app.h"
#include "game_menu.h"
#include "global_world.h"
#include "location.h"
#include "level_file.h"
#include "renderer.h"

class NewProfileButton : public DarwiniaButton
{

  void MouseUp() override
  {
    auto parent = static_cast<NewUserProfileWindow*>(m_parent);
    g_app->SetProfileName(parent->s_profileName);
    g_app->m_globalWorld->m_loadingNewProfile = true;
    g_app->LoadProfile();

    EclRemoveWindow(m_parent->m_name);
    EclRemoveWindow("dialog_newprofile");
    EclRemoveWindow("dialog_mainmenu");

  }
};

class LoadUserProfileButton : public GameMenuButton
{
  protected:
    char m_profileName[256];

  public:
    LoadUserProfileButton()
      : GameMenuButton(UnicodeString()) { m_profileName[0] = '\0'; }

    void SetShortProperties(const char* _name, int x, int y, int w, int h, const UnicodeString& _caption,
                            const UnicodeString& _tooltip = UnicodeString()) override
    {
      int length = wcslen(_caption.m_unicodestring);
      auto caption = new wchar_t[length + 5];
      wcscpy(caption, _caption.m_unicodestring);
      wchar_t* c = wcsrchr(caption, L'-');
      if (c != nullptr)
      {
        c++;
        wchar_t* stopPoint;
        long num = wcstol(c, &stopPoint, 10) + 1;
        swprintf(c, 5, L"%d", num);
        UnicodeString newCaption(caption);
        DarwiniaButton::SetShortProperties(_name, x, y, w, h, newCaption, _tooltip);
      }
      else
        DarwiniaButton::SetShortProperties(_name, x, y, w, h, _caption, _tooltip);
      delete [] caption;
    }

    void SetProfileName(const char* _profileName) { strcpy(m_profileName, _profileName); }
};

class NewProfileWindowButton : public DarwiniaButton
{
  public:
    void MouseUp() override { EclRegisterWindow(new NewUserProfileWindow(), m_parent); }
};

ResetProfileButton::ResetProfileButton()
  : GameMenuButton(UnicodeString()) {}

void ResetProfileButton::MouseUp()
{
  if (m_inactive)
    return;
  char saveDir[256];
  sprintf(saveDir, "users/%s/", g_app->m_userProfileName);
  auto allFiles = ListDirectory(saveDir, "*.*");

  for (int i = 0; i < allFiles.size(); ++i)
    DeleteThisFile(allFiles[i].c_str());
  g_app->LoadProfile();
  EclRemoveWindow(m_parent->m_name);
  EclRemoveWindow("dialog_mainmenu");
}

UserProfileWindow::UserProfileWindow()
  : GameOptionsWindow("dialog_profile") {}

void UserProfileWindow::Render(bool hasFocus) { GameOptionsWindow::Render(hasFocus); }

void UserProfileWindow::Create()
{
  char profileDir[256];
  sprintf(profileDir, "%susers/*.*", g_app->GetProfileDirectory());
  auto profileList = ListSubDirectoryNames(profileDir);

  int w = g_app->m_renderer->ScreenW();
  int h = g_app->m_renderer->ScreenH();

  SetMenuSize(w, h);
  SetPosition(0, 0);
  SetMovable(false);

  GameOptionsWindow::Create();

  //
  // New profile

  float leftX, leftY, leftW, leftH;
  float fontLarge, fontMed, fontSmall;
  float buttonW, buttonH, gap;
  GetPosition_LeftBox(leftX, leftY, leftW, leftH);
  GetFontSizes(fontLarge, fontMed, fontSmall);
  GetButtonSizes(buttonW, buttonH, gap);

  float x = leftX + (leftW - buttonW) / 2.0;
  float y = leftY + buttonH;
  float fontSize = fontMed;
  float border = gap + buttonH;

  y -= border;

  //int invertY = y+GetMenuSize(20);

  //
  // Inverted box

  /*if( numProfiles > 0 )
  {
      InvertedBox *box = new InvertedBox();
      box->SetShortProperties( "Box", 10, invertY, m_w-20, m_h - invertY - GetMenuSize(60), UnicodeString("Box") );
      RegisterButton( box );
  }*/

  //
  // Load existing profile button

  /*for( int i = 0; i < profileList->Size(); ++i )
  {
      UnicodeString profileName(profileList->GetData(i));
      wchar_t caption[256];
  swprintf( caption, L"%s: '%s'", LANGUAGEPHRASE("dialog_loadprofile").m_unicodestring, profileName.m_unicodestring );
      LoadUserProfileButton *button = new LoadUserProfileButton();
      button->SetShortProperties( "dialog_loadprofile", 20, y+=h, m_w-40, GetMenuSize(20), UnicodeString(caption) );
      button->m_profileName = profileList->GetData(i);
      button->m_fontSize = GetMenuSize(11);
      button->m_centered = true;
      RegisterButton( button );
      m_buttonOrder.PutData( button );
  }
  delete profileList;
  

  NewProfileWindowButton *newProfile = new NewProfileWindowButton();
  newProfile->SetShortProperties( "dialog_newprofile", 10, m_h - GetMenuSize(55), m_w - 20, GetMenuSize(20), LANGUAGEPHRASE("dialog_newprofile") );
  newProfile->m_fontSize = GetMenuSize(13);
  newProfile->m_centered = true;
  RegisterButton( newProfile );
  m_buttonOrder.PutData( newProfile );*/

  bool currentlyAAA = (strcmp(g_app->m_userProfileName, "AccessAllAreas") == 0);
  DebugTrace("%sCurrently AAA\n", currentlyAAA ? "" : "!");

  if (g_prefsManager->GetInt("AccessAllAreasUnlocked") == 1)
  {
    // if we have the accessallareas option available, and dont currently have it loaded, create the button
    auto aaa = new LoadUserProfileButton();
    aaa->SetShortProperties("accessallareas", x, y += border, buttonW, buttonH, LANGUAGEPHRASE("taskmanager_accessallareas"));
    aaa->SetProfileName(_strdup("AccessAllAreas"));
    aaa->m_fontSize = fontSize;
    aaa->m_inactive = currentlyAAA;
    RegisterButton(aaa);
    m_buttonOrder.PutData(aaa);

    if (currentlyAAA)
    {
      aaa->m_inactive = true;
      UnicodeString newCaption(LANGUAGEPHRASE("dialog_profile_loaded"));
      newCaption.ReplaceStringFlag(L'U', aaa->m_caption);
      aaa->SetCaption(newCaption);
    }
  }

  auto reset = new ConfirmResetButton();
  reset->SetShortProperties("reset", x, y += border, buttonW, buttonH, LANGUAGEPHRASE("dialog_resetprofile"));
  reset->m_fontSize = fontSize;
  reset->m_inactive = currentlyAAA;
  RegisterButton(reset);
  m_buttonOrder.PutData(reset);

  int buttonY = (leftY + leftH) - (buttonH * 2.0f) - buttonH * 0.3f;

  auto cancel = new MenuCloseButton("dialog_close");
  cancel->SetShortProperties("dialog_close", x, buttonY + buttonH + 5, buttonW, buttonH, LANGUAGEPHRASE("dialog_close"));
  cancel->m_fontSize = fontSize;
  RegisterButton(cancel);
  m_buttonOrder.PutData(cancel);
  m_backButton = cancel;

}


char NewUserProfileWindow::s_profileName[256] = "NewUser";

NewUserProfileWindow::NewUserProfileWindow()
  : DarwiniaWindow("dialog_newprofile") {}

void NewUserProfileWindow::Create()
{
  SetMenuSize(300, 110);
  SetPosition(g_app->m_renderer->ScreenW() / 2 - m_w / 2, g_app->m_renderer->ScreenH() / 2 - m_h / 2);

  DarwiniaWindow::Create();

  auto box = new InvertedBox();
  box->SetShortProperties("box", 10, GetMenuSize(30), m_w - 20, GetMenuSize(40), UnicodeString("box"));
  RegisterButton(box);

  CreateValueControl("dialog_name", s_profileName, GetMenuSize(40), 0, 0, sizeof(s_profileName), nullptr, 20, m_w - 40);

  int y = m_h - GetMenuSize(30);

  auto close = new CloseButton();
  close->SetShortProperties("dialog_cancel", 10, y, m_w / 2 - 15, GetMenuSize(20), LANGUAGEPHRASE("dialog_cancel"));
  close->m_fontSize = GetMenuSize(12);
  RegisterButton(close);

  auto newProfile = new NewProfileButton();
  newProfile->SetShortProperties("dialog_create", close->m_x + close->m_w + 10, y, m_w / 2 - 15, GetMenuSize(20),
                                 LANGUAGEPHRASE("dialog_create"));
  newProfile->m_fontSize = GetMenuSize(12);
  RegisterButton(newProfile);
}
