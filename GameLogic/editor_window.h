#ifndef _included_EditWindow_h
#define _included_EditWindow_h

#ifdef LOCATION_EDITOR

#include "darwinia_window.h"


class MainEditWindow : public DarwiniaWindow
{
public:
	DarwiniaWindow *m_currentEditWindow;

	MainEditWindow( char *name );

	void Create();
};

#endif // LOCATION_EDITOR


#endif
