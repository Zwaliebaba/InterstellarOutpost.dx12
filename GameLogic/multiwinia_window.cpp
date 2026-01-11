#include "pch.h"
#include "language_table.h"
#include "text_renderer.h"
#include "filesys_utils.h"
#include "multiwinia_window.h"
#include "input_field.h"
#include "multiwinia_filedialog.h"
#include "app.h"
#include "level_file.h"
#include "multiwinia.h"
#include "main.h"

class CreateMapButton : public GameMenuButton
{
public:
    CreateMapButton()
    :   GameMenuButton(UnicodeString())
    {
    }

    void MouseUp()
    {
        NewMapWindow *parent = (NewMapWindow *) m_parent;
        char filename[512];
        TextReader *reader = NULL;

        if( !IsDirectory(g_app->GetMapDirectory()) ) 
        {
            CreateDirectory(g_app->GetMapDirectory());
        }
        sprintf( filename, "%s%s", g_app->GetMapDirectory(), parent->m_mapFilename.Get() );
        reader = new TextFileReader(filename);

        if( DoesFileExist(filename) )
        //if( reader )
        {
            delete reader;
            parent->CreateErrorDialogue(LANGUAGEPHRASE("multiwinia_editor_mapexists"));
            return;
        }

        LevelFile levelFile;
        sprintf( levelFile.m_mapFilename, "%s", parent->m_mapFilename.Get() );      
        sprintf( levelFile.m_missionFilename, "null" );
        strlwr( levelFile.m_mapFilename );

        levelFile.Save();

        UnicodeString message = LANGUAGEPHRASE("multiwinia_editor_mapcreated");
        message.ReplaceStringFlag( 'F', parent->m_mapFilename.Get() );
        parent->CreateErrorDialogue(message, true);
    }
};

class SelectMapButton : public GameMenuButton
{
public:    

    char m_filter[64];
    char m_path[256];

    SelectMapButton()
    :   GameMenuButton(UnicodeString())
    {
        strcpy( m_filter, "mp_*" );
        strcpy( m_path, "levels/" );
    }

    void MouseUp()
    {
        g_app->m_soundSystem->TriggerOtherEvent( NULL, "MenuSelect", SoundSourceBlueprint::TypeMultiwiniaInterface );
        g_app->m_renderer->InitialiseMenuTransition(1.0f, 1);
        MultiwiniaFileDialog *fileDialog = new MultiwiniaFileDialog( "Select File", m_parent->m_name, m_path, m_filter, false );    
        EclRegisterWindow( fileDialog );
        g_app->m_doMenuTransition = true;
    }
};

class NewMapWindowButton : public GameMenuButton
{
public:
    NewMapWindowButton()
    :   GameMenuButton(UnicodeString())
    {
    }

    void MouseUp()
    {
        g_app->m_soundSystem->TriggerOtherEvent( NULL, "MenuSelect", SoundSourceBlueprint::TypeMultiwiniaInterface );
        g_app->m_renderer->InitialiseMenuTransition(1.0f, 1);
        EclRegisterWindow( new NewMapWindow() );
        g_app->m_doMenuTransition = true;
    }
};

MultiwiniaWindow::MultiwiniaWindow()
:   GameOptionsWindow( "dialog_multiwinia" ),
    m_gameType(0),
    m_mapFilename( m_fileString, 256 )
{
    int screenW = g_app->m_renderer->ScreenW();
    int screenH = g_app->m_renderer->ScreenH();

    SetMenuSize( screenW, screenH );
    SetPosition( 0, 0 );
    SetMovable( false );

    m_mapFilename.Set("mp_blank.txt" );

    for( int i = 0; i < GlobalResearch::NumResearchItems; ++i )
    {
        m_researchLevel[i] = 3;
    }    
}

void MultiwiniaWindow::Create()
{
    GameOptionsWindow::Create();

    SetTitle( LANGUAGEPHRASE( "multiwinia_mainmenu_title" ) );

    float leftX, leftY, leftW, leftH;
    float fontLarge, fontMed, fontSmall;
    float buttonW, buttonH, gap;
    GetPosition_LeftBox(leftX, leftY, leftW, leftH );
    GetFontSizes( fontLarge, fontMed, fontSmall );
    GetButtonSizes( buttonW, buttonH, gap );

    float x = leftX + (leftW-buttonW)/2.0f;
    float y = leftY;
    float fontSize = fontMed;

    y += buttonH * 0.5f;
    GameMenuTitleButton *title = new GameMenuTitleButton();
    title->SetShortProperties( "title", leftX+10, leftY+10, leftW-20, buttonH*1.5f, LANGUAGEPHRASE("taskmanager_mapeditor" ) );
    title->m_fontSize = fontMed;
    RegisterButton( title );
    y += buttonH*1.3f;

    SelectMapButton *selectMap = new SelectMapButton();
    selectMap->SetShortProperties( "SelectMap", x, y+=buttonH+gap, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_editor_edit") );
    selectMap->m_fontSize = fontSize;
    strcpy(selectMap->m_filter, "*.mwm");
    sprintf(selectMap->m_path, "%s", g_app->GetMapDirectory() );
    RegisterButton( selectMap );
    m_buttonOrder.PutData( selectMap );

    NewMapWindowButton *newmap = new NewMapWindowButton();
    newmap->SetShortProperties("newmapbutton", x, y+=buttonH+gap, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_editor_create") );
    newmap->m_fontSize = fontSize;
    RegisterButton( newmap );
    m_buttonOrder.PutData( newmap );

    //buttonH /= 0.65f;
    float yPos = leftY + leftH - buttonH * 2;

    MenuCloseButton *close = new MenuCloseButton("dialog_back");
    close->SetShortProperties( "dialog_close", x, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back") );
    close->m_fontSize = fontMed;
    RegisterButton( close );
	m_buttonOrder.PutData( close );
	m_backButton = close;
}

void MultiwiniaWindow::Update()
{
    if( strcmp( m_mapFilename.Get(), "mp_blank.txt" ) != 0 )
    {
        char filename[512];
        TextReader *reader = NULL;

//#ifdef TARGET_DEBUG
        sprintf( filename, "levels/%s", m_mapFilename.Get() );
        reader = g_app->m_resource->GetTextReader( filename );
//#else
        if( !reader )
        {
            sprintf( filename, "%s%s", g_app->GetMapDirectory(), m_mapFilename.Get() );
            reader = new TextFileReader( filename );
        }
//#endif

        if( reader )
        {
            delete reader; 

            strcpy( g_app->m_requestedMap, m_mapFilename.Get() );        
            strcpy( g_app->m_requestedMission, "null" );

            g_app->m_requestToggleEditing = true;
            g_app->m_requestedLocationId = 999;

            RemoveAllWindows();
        }
    }
}



NewMapWindow::NewMapWindow()
:   GameOptionsWindow("new_map"),
    m_mapFilename( m_fileString, 256 )
{
    m_mapFilename.Set( "mp_newmap.txt" );
}

void NewMapWindow::Create()
{
    GameOptionsWindow::Create();

    SetTitle( LANGUAGEPHRASE( "multiwinia_mainmenu_title" ) );

    int w = g_app->m_renderer->ScreenW();
    int h = g_app->m_renderer->ScreenH();

    float leftX, leftY, leftW, leftH;
    float fontLarge, fontMed, fontSmall;
    float buttonW, buttonH, gap;
    GetPosition_LeftBox(leftX, leftY, leftW, leftH );
    GetFontSizes( fontLarge, fontMed, fontSmall );
    GetButtonSizes( buttonW, buttonH, gap );

    float x = leftX + (leftW-buttonW)/2.0f;
    float y = leftY + buttonH;
    float fontSize = fontSmall;

    y += buttonH * 0.5f;
    GameMenuTitleButton *title = new GameMenuTitleButton();
    title->SetShortProperties( "title", leftX+10, leftY+10, leftW-20, buttonH*1.5f, LANGUAGEPHRASE("multiwinia_editor_create" ) );
    title->m_fontSize = fontMed;
    RegisterButton( title );
    y += buttonH;

    buttonH *= 0.65f;

    CreateMenuControl( "multiwinia_editor_mapfilename", InputField::TypeString, &m_mapFilename, y+=buttonH+gap, 0, NULL, x, buttonW, fontSize );
    m_buttonOrder.PutData( GetButton("multiwinia_editor_mapfilename") );

    CreateMapButton *createMap = new CreateMapButton();
    createMap->SetShortProperties( "Create", x, y+=buttonH+gap, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_editor_create") );
    createMap->m_fontSize = fontSize;
    RegisterButton( createMap );
    m_buttonOrder.PutData( createMap );

    buttonH /= 0.65f;
    float yPos = leftY + leftH - buttonH * 2;

    MenuCloseButton *close = new MenuCloseButton("dialog_back");
    close->SetShortProperties( "dialog_close", x, yPos, buttonW, buttonH, LANGUAGEPHRASE("multiwinia_menu_back") );
    close->m_fontSize = fontMed;
    //close->m_centered = true;
    RegisterButton( close );
	m_buttonOrder.PutData( close );
	m_backButton = close;
}