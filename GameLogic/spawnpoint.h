#ifndef _included_spawnpoint_h
#define _included_spawnpoint_h

#include "building.h"

class SpawnBuildingSpirit
{
  public:
    SpawnBuildingSpirit();

    int m_targetBuildingId;
    double m_currentProgress;
};

class SpawnBuildingLink
{
  public:
    SpawnBuildingLink();
    ~SpawnBuildingLink();

    int m_targetBuildingId; // Who I am directly linked to

    LList<int> m_targets; // List of all SpawnPoint buildings reachable down this route
    LList<SpawnBuildingSpirit*> m_spirits;
};

class SpawnBuilding : public Building
{
  protected:
    LList<SpawnBuildingLink*> m_links;
    ShapeMarker* m_spiritLink;

    Vector3 m_visibilityMidpoint;
    double m_visibilityRadius;

  public:
    SpawnBuilding();
    ~SpawnBuilding() override;

    void Initialise(Building* _template) override;
    bool Advance() override;

    virtual void TriggerSpirit(SpawnBuildingSpirit* _spirit, double progress = 0.0);

    bool IsInView() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
    void RenderSpirit(const Vector3& _pos, int teamId);

    Vector3 GetSpiritLink();
    void SetBuildingLink(int _buildingId) override;
    void ClearLinks();

    LList<int>* ExploreLinks(); // Returns a list of all SpawnBuildings accessable

    static bool IsSpawnBuilding(int _type);

    void Read(TextReader* _in, bool _dynamic) override;
};

class SpawnLink : public SpawnBuilding
{
  public:
    SpawnLink();
};

class MasterSpawnPoint : public SpawnBuilding
{
  protected:
    static int s_masterSpawnPointId;
    bool m_exploreLinks;

  public:
    MasterSpawnPoint();

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    void RequestSpirit(int _targetBuildingId);

    void GetObjectiveCounter(UnicodeString& _dest) override;

    static MasterSpawnPoint* GetMasterSpawnPoint();
};

class SpawnPoint : public SpawnBuilding
{
  protected:
    bool PopulationLocked();

    double m_evaluateTimer;
    double m_spawnTimer;
    int m_populationLock; // Building ID (if found), -1 = not yet searched, -2 = nothing found
    int m_numFriendsNearby;
    ShapeMarker* m_doorMarker;
    int m_spawnCounter;
    int m_numSpawned;

  public:
    SpawnPoint();

    bool Advance() override;
    void Initialise(Building* _template) override;

    bool PerformDepthSort(Vector3& _centrePos) override;

    void TriggerSpirit(SpawnBuildingSpirit* _spirit, double progress = 0.0) override;
    void SpawnDarwinian();

    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
    void RenderPorts() override;

    static double CalculateHandicap(int _teamId);
    void RecalculateOwnership();

    static int s_numSpawnPoints[NUM_TEAMS + 1];
    // tracks the number of spawn points each team owns. used to calculate handicap. you must query your team id + 1 instead of the team id itself to get a valid result
    double m_activationTimer;

};

class SpawnPopulationLock : public Building
{
  public:
    double m_searchRadius;
    int m_maxPopulation;
    int m_teamCount[NUM_TEAMS];

  protected:
    static double s_overpopulationTimer;
    static int s_overpopulation;
    int m_originalMaxPopulation;
    double m_recountTimer;
    int m_recountTeamId;

  public:
    SpawnPopulationLock();

    void Initialise(Building* _template) override;
    bool Advance() override;

    void Read(TextReader* _in, bool _dynamic) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;

    static void Reset();
};

#endif
