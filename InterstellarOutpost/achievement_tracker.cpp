#include "pch.h"
#include "resource.h"
#include "filesys_utils.h"
#include "text_file_writer.h"
#include "text_stream_readers.h"
#include "achievement_tracker.h"
#include "location.h"
#include "level_file.h"
#include "app.h"

#define MAX_LEVEL_ID 49

#ifdef    INCLUDEGAMEMODE_ASSAULT
#define VALID_ACHIEVEMENT_LEVELS 49
#else
#define VALID_ACHIEVEMENT_LEVELS 41
#endif 


AchievementTracker::AchievementTracker()
:   m_isMasterPlayer(false),
	m_hasLoaded(false),
	m_totalKills(-1)
{
    memset( m_won, 0, sizeof(m_won) );
    memset( m_visited, 0, sizeof(m_visited) );

	TextReader *in = g_app->m_resource->GetTextReader( ACHIEVEMENT_LEVEL_ENUM );
	if( !in )
	{
		DebugTrace("Failed to load level_enum.txt\n");
	}
	else if( *in )
	{
		while( in->ReadLine() )
		{
			if( !in->TokenAvailable() ) continue;
			char *word = in->GetNextToken();
			int  value = atoi( in->GetNextToken() );

			m_levelList.PutData( word, value );
			m_levelListReverse[value] = word;
		}
	}
	delete in;
}

unsigned char AchievementTracker::GetViralAchievementMask()
{
	unsigned char ret = 0;
	if( m_isMasterPlayer ) ret |= VIRAL_MASTER_ACHIEVEMENT_MASK;
	DebugTrace("Viral achievement mask is %d\n", ret);
	return ret;
}

void AchievementTracker::Load()
{
}

bool AchievementTracker::HasLoaded()
{
	return m_hasLoaded;
}

void AchievementTracker::AddToTotalKills(int kills)
{
	m_totalKills += kills;
	if( m_totalKills >= 1000000 ) g_app->GiveAchievement(App::DarwiniaAchievementGenocide);
	Save();
}

void AchievementTracker::Save()
{
}

void AchievementTracker::AddVictoryOnLevel(char *_mapName)
{
    if( AlreadyWonLevel( _mapName ) ) return;

    int levelId = m_levelList.GetData( _mapName, -1 );  
    if( levelId == -1 ) return;
    if( levelId > MAX_LEVEL_ID ) return;

    int index = levelId / 8;
    int mod = levelId % 8;
    m_won[index] |= (1 << mod);

    Save();

    if( NumWonLevels() == VALID_ACHIEVEMENT_LEVELS )
    {
        g_app->GiveAchievement( App::DarwiniaAchievementMasterOfMultiwinia );
    }
}

void AchievementTracker::AddVisitedLevel(char *_mapName)
{
    if( AlreadyVisitedLevel( _mapName ) ) return;

    int levelId = m_levelList.GetData( _mapName, -1 );  
    if( levelId == -1 ) return;
    if( levelId > MAX_LEVEL_ID ) return;

    int index = levelId / 8;
    int mod = levelId % 8;
    m_visited[index] |= (1 << mod);

    Save();

    if( NumVisitedLevels() == VALID_ACHIEVEMENT_LEVELS )
    {
        g_app->GiveAchievement( App::DarwiniaAchievementExplorer );
    }
}

bool AchievementTracker::AlreadyWonLevel( char *_mapName )
{
    int levelId = m_levelList.GetData( _mapName, -1 );  
    if( levelId == -1 ) return false;

    return (m_won[ levelId / 8 ] >> (levelId % 8)) & 1;
}

bool AchievementTracker::AlreadyVisitedLevel(char *_mapName)
{
    int levelId = m_levelList.GetData( _mapName, -1 );  
    if( levelId == -1 ) return false;

    return (m_visited[ levelId / 8 ] >> (levelId % 8)) & 1;
}

void AchievementTracker::SetNumLevels( int _numLevels )
{}

int AchievementTracker::NumWonLevels()
{
    int num = 0;
    for( int i = 0; i < 8; ++i )
    {
        for( int j = 0; j < 8; ++j )
        {
            if( ( m_won[i] >> j ) & 1 ) num++;
        }
    }

    return num;
}

int AchievementTracker::NumVisitedLevels()
{
    int num = 0;
    for( int i = 0; i < 8; ++i )
    {
        for( int j = 0; j < 8; ++j )
        {
            if( ( m_visited[i] >> j ) & 1 ) num++;
        }
    }

    return num;
}

