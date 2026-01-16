#ifndef _included_virii_h
#define _included_virii_h

#include "entity.h"
#include "unit.h"
#include "globals.h"

#define VIRII_MAXSEARCHRANGE    60.0
#define VIRII_MINSEARCHRANGE    30.0
#define VIRII_TAILLENGTH        175.0

#define VIRII_POPULATIONLIMIT   250

class ViriiHistory;

//*****************************************************************************
// Class ViriiUnit
//*****************************************************************************

class ViriiUnit : public Unit
{
  public:
    bool m_enemiesFound;
    bool m_cameraClose;

    ViriiUnit(int teamId, int unitId, int numEntities, const Vector3& _pos);

    bool Advance(int _slice) override;
    void Render(double _predictionTime) override;
};

//*****************************************************************************
// Class Virii
//*****************************************************************************

class Virii : public Entity
{
  public:
    enum
    {
      StateIdle,
      StateAttacking,
      StateToSpirit,
      StateToEgg
    };

    int m_state;

    double m_hoverHeight;
    double m_retargetTimer;
    WorldObjectId m_enemyId;
    WorldObjectId m_eggId;
    int m_spiritId;
    Vector3 m_wayPoint;
    double m_historyTimer;

    Vector3 m_prevPos;
    double m_prevPosTimer;

    static int s_viriiPopulation[NUM_TEAMS];

  protected:
    bool SearchForEnemies();
    bool SearchForSpirits();
    bool SearchForEggs();
    bool SearchForIdleDirection();

    WorldObjectId FindNearbyEgg(int _spiritId, double _autoAccept = 99999.9);
    WorldObjectId FindNearbyEgg(const Vector3& _pos);

    bool AdvanceToTargetPos(const Vector3& _pos); // returns have-I-Arrived?    
    void RecordHistoryPosition(bool _required); // if !_required this is simply to make it smoother
    Vector3 AdvanceDeadPositionVector(int _index, const Vector3& _pos, double _time);

    LList<ViriiHistory*> m_positionHistory;

  public:
    Virii();
    ~Virii() override;

    void Begin() override;

    bool Advance(Unit* _unit) override;
    bool AdvanceIdle();
    bool AdvanceAttacking();
    bool AdvanceToSpirit();
    bool AdvanceToEgg();
    bool AdvanceDead();

    bool IsInView() override;

    void Render(double predictionTime, int teamId, int _detail);
};

//*****************************************************************************
// Class ViriiHistory
//*****************************************************************************

class ViriiHistory
{
  public:
    Vector3 m_pos; // Position in world
    Vector3 m_right; // Right vector (front is to next point, up is land normal)
    Vector3 m_glowDiff; // Diff to previous history point, sized for glow effect
    double m_distance; // Distance to previous history point
    bool m_required; // True means this is an absolute history position (eg direction change)
    // false means its just to smooth out the path (eg height change)
};

#endif
