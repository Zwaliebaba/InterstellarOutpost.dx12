#ifndef _included_debugmenu_h
#define _included_debugmenu_h

#include "darwinia_window.h"
#include "app.h"


class DebugMenu : public DarwiniaWindow
{
public:
    DebugMenu(const char *name );

	void Advance();
    void Create();
	void Render(bool hasFocus);
};


class DebugKeyBindings
{
public:
	static void DebugMenu();
	static void NetworkButton();
	static void DebugCameraButton();
	static void FollowCameraButton();
	static void FPSButton();
	static void InputLogButton();
	static void ProfileButton();
#ifdef LOCATION_EDITOR
	static void EditorButton();
#endif
#ifdef CHEATMENU_ENABLED
    static void CheatButton();
#endif
	static void ReallyQuitButton();
	static void ToggleFullscreenButton();
};


#endif
