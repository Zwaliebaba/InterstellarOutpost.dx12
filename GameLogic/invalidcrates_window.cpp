#include "pch.h"
#include "language_table.h"
#include "text_renderer.h"

#include "checkbox.h"
#include "invalidcrates_window.h"

#include "app.h"
#include "level_file.h"
#include "location.h"

#include "crate.h"

InvalidCratesWindow::InvalidCratesWindow()
:   DarwiniaWindow("InvalidCratesWindow")
{
}

void InvalidCratesWindow::Create()
{
    DarwiniaWindow::Create();

	int height = 5;
	int pitch = 17;
    int buttonWidth = m_w - 20;

    CheckBox *cb = NULL;

    for( int i = 0; i < Crate::NumCrateRewards; ++i )
    {
        char name[64];
        sprintf( name, "crate_%d", i );
        UnicodeString trans;
        Crate::GetNameTranslated( i, trans );
        cb = new CheckBox();
        cb->SetShortProperties( name, 10, height+=pitch, m_w - 20, -1, trans );
        cb->RegisterBool( &g_app->m_location->m_levelFile->m_invalidCrates[i] );
        RegisterButton( cb );
    }

}