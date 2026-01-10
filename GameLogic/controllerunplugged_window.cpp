#include "pch.h"
#include "text_renderer.h"

#include "controllerunplugged_window.h"
#include "mainmenus.h"


ControllerUnpluggedWindow::ControllerUnpluggedWindow()
:   GameOptionsWindow( "controller_unplugged" ),
    m_dialogCreated(false)
{
}

void ControllerUnpluggedWindow::Update()
{
    if( !m_dialogCreated )
    {
        m_dialogCreated = true;
        CreateErrorDialogue( LANGUAGEPHRASE("dialog_unplugged_pc") );
    }

    if( !m_showingErrorDialogue ||
        g_inputManager->controlEvent( ControlMenuEscape ) )
    {
        EclRemoveWindow( m_name );
    }
}

void ControllerUnpluggedWindow::Render(bool _hasFocus)
{
    if( m_showingErrorDialogue )
    {
        RenderErrorDialogue();
    }
}