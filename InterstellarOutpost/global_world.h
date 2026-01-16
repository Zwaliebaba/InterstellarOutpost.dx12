#ifndef _included_global_world_h
#define _included_global_world_h

#include "llist.h"
#include "sphere_renderer.h"
#include "unicode_string.h"

class TextReader;
class Vector3;
class Building;
class GlobalInternet;

class GlobalLocation
{
  public:
    int m_id;
    Vector3 m_pos;
    bool m_available; // Is it connected on the transit system

    char m_name[256];
    char m_mapFilename[256];
    char m_missionFilename[256];
    bool m_missionCompleted;

    int m_numSpirits; // Number of spirits that have died

    GlobalLocation();

    void AddSpirits(int _count = 1);
};

// ****************************************************************************
// GlobalBuilding
// ****************************************************************************

class GlobalBuilding
{
  public:
    int m_id;
    int m_teamId;
    int m_locationId;
    Vector3 m_pos;
    int m_type;
    bool m_online;
    int m_link;
    Shape* m_shape;

    GlobalBuilding();
};

// ****************************************************************************
// Class GlobalEvent + guests
// ****************************************************************************

class GlobalEventCondition
{
  public:
    enum
    {
      AlwaysTrue,
      // 0
      BuildingOnline,
      // 1
      BuildingOffline,
      // 2
      ResearchOwned,
      // 3
      NotInLocation,
      // 4
      DebugKey,
      // 5
      NeverTrue,
      // 6
      NumConditions // Remember to update GetTypeName
    };

    int m_type;
    int m_id;
    int m_locationId;
    char* m_stringId; // Brief description
    char* m_cutScene; // Filename of cutscene to run

    GlobalEventCondition();
    GlobalEventCondition(const GlobalEventCondition& _other);
    ~GlobalEventCondition();

    bool Evaluate();

    void SetStringId(char* _stringId);
    void SetCutScene(char* _cutScene);

    static char* GetTypeName(int _type);
    static int GetType(const char* _typeName);
};

class GlobalEventAction
{
  public:
    enum
    {
      SetMission,
      RunScript,
      MakeAvailable,
      NumActionTypes
    };

    int m_type;
    int m_locationId;
    char m_filename[256];

    GlobalEventAction();

    void Read(TextReader* _in);
    void Execute();

    static char* GetTypeName(int _type);
};

class GlobalEvent
{
  public:
    LList<GlobalEventCondition*> m_conditions;
    LList<GlobalEventAction*> m_actions;

    GlobalEvent();
    GlobalEvent(GlobalEvent& _other); // Copy constructor only used by TestHarness

    void Read(TextReader* _in);
    bool Evaluate();
    bool Execute(); // Returns true when all done

    void MakeAlwaysTrue();
};

// ****************************************************************************
// Class GlobalResearch
// ****************************************************************************

#define GLOBALRESEARCH_TIMEPERPOINT             10
#define GLOBALRESEARCH_POINTS_CONTROLTOWER      22

class GlobalResearch
{
  public:
    enum
    {
      TypeDarwinian,
      TypeOfficer,
      TypeSquad,
      TypeLaser,
      TypeGrenade,
      TypeRocket,
      TypeController,
      TypeAirStrike,
      TypeArmour,
      TypeTaskManager,
      TypeEngineer,
      TypeHarvester,
      TypeNuke,
      TypeSubversion,
      TypeBooster,
      TypeHotFeet,
      TypeGunTurret,
      TypeShaman,
      TypeInfection,
      TypeMagicalForest,
      TypeDarkForest,
      TypeAntsNest,
      TypePlague,
      TypeEggSpawn,
      TypeMeteorShower,
      TypeExtinguisher,
      TypeRage,
      TypeTank,
      TypeDropShip,
      NumResearchItems
    };

    int m_researchLevel[NumResearchItems];
    int m_researchProgress[NumResearchItems];
    int m_currentResearch;
    int m_researchPoints;
    double m_researchTimer;

    GlobalResearch();

    bool HasResearch(int _type);
    int CurrentProgress(int _type);
    int CurrentLevel(int _type);

    void AddResearch(int _type);
    void SetCurrentProgress(int _type, int _progress);

    void IncreaseProgress(int _amount);
    void DecreaseProgress(int _amount);
    int RequiredProgress(int _level); // Progress required to reach this level

    void EvaluateLevel(int _type);

    void SetCurrentResearch(int _type);
    void GiveResearchPoints(int _numPoints);
    void AdvanceResearch();

    void Read(TextReader* _in);

    static char* GetTypeName(int _type);
    static int GetType(char* _name);

    static void GetTypeNameTranslated(int _type, UnicodeString& _dest);
};

// ****************************************************************************
// Class SphereWorld
// ****************************************************************************

class SphereWorld
{
  public:
    Shape* m_shapeOuter;
    Shape* m_shapeMiddle;
    Shape* m_shapeInner;

    int m_numLocations;
    LList<double>* m_spirits; // An array with one LList<double> per location

    SphereWorld();

    void AddLocation(int _locationId);

    void Render();
    void RenderWorldShape();
    void RenderIslands();
    void RenderTrunkLinks();
    void RenderHeaven();
    void RenderSpirits();
};

// ****************************************************************************
// Class GlobalWorld
// ****************************************************************************

class GlobalWorld
{
  public:
    GlobalInternet* m_globalInternet;
    SphereWorld* m_sphereWorld;
    GlobalResearch* m_research;

    LList<GlobalLocation*> m_locations;
    LList<GlobalBuilding*> m_buildings;
    LList<GlobalEvent*> m_events;
    int m_myTeamId;

    int m_editorMode;
    int m_editorSelectionId;
    unsigned int m_spLevelTimes[11];
    int m_spSquaddiesUsed[11];

    bool m_loadingNewProfile;

    static int s_nextUniqueBuildingId;

  protected:
    void ParseLocations(TextReader* _in);
    void ParseBuildings(TextReader* _in);
    void ParseEvents(TextReader* _in);
    void ParseLevelTimes(TextReader* _in);
    void ParseSquaddiesUsed(TextReader* _in);

    void AddLevelBuildingToGlobalBuildings(Building* _building, int _locId);

    int m_nextLocationId;
    int m_nextBuildingId;

    int m_locationRequested; // Stores the location a user has clicked on while we fade out. -1 means no request yet.

  public:
    GlobalWorld();
    GlobalWorld(GlobalWorld&); // Copy constructor only used in TestHarness
    ~GlobalWorld();

    void Advance();
    void Render();

    int LocationHit(const Vector3& _pos, const Vector3& _dir, double locationRadius = 5000.0);

    void AddLocation(GlobalLocation* location);
    void AddBuilding(GlobalBuilding* building);

    GlobalLocation* GetLocation(int _id);
    GlobalLocation* GetLocation(const char* _name);
    GlobalLocation* GetHighlightedLocation(); // ie whats under the mouse
    int GetLocationId(const char* _name);
    int GetLocationIdFromMapFilename(const char* _mapFilename);
    char* GetLocationName(int _id);
    void GetLocationNameTranslated(int _id, UnicodeString& _dest);
    Vector3 GetLocationPosition(int _id);

    GlobalBuilding* GetBuilding(int _id, int _locationId);
    int GenerateBuildingId();

    bool EvaluateEvents(); // Returns true if an event was triggered
    void TransferSpirits(int _locationId);

    void LoadGame(char* _filename);

    void LoadLocations(const char* _filename);

    void SetupLights();
    void SetupFog();

    double GetSize();
};

#endif
