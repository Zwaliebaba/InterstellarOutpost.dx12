#ifndef _included_pokeywindow_h
#define _included_pokeywindow_h

#ifdef SOUND_EDITOR

#include "interface/darwinia_window.h"


struct FSOUND_STREAM;

class PokeyWindow : public DarwiniaWindow
{
private:
	FSOUND_STREAM *m_stream;
	int m_selectionType;
	int m_selectionId;

public:
    PokeyWindow( char *name );
	~PokeyWindow();

	void Create();

	void Advance();
    void Render( bool hasFocus );
};

#endif // SOUND_EDITOR

#endif
