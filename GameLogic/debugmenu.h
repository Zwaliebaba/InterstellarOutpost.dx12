#ifndef _included_debugmenu_h
#define _included_debugmenu_h

#include "darwinia_window.h"
#include "app.h"

class DebugKeyBindings
{
public:
    static void NetworkButton();
	static void DebugCameraButton();
	static void FPSButton();
	static void ProfileButton();
#ifdef CHEATMENU_ENABLED
    static void CheatButton();
#endif
};


#endif
