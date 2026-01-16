#ifndef INCLUDED_INSERTION_SQUAD
#define INCLUDED_INSERTION_SQUAD

#include "unit.h"

#define GAP_BETWEEN_MEN	10.0

//*****************************************************************************
// Class HistoricWayPoint
// 
// Stores positions where the user clicked.
//*****************************************************************************

class HistoricWayPoint
{
  public:
    Vector3 m_pos;
    unsigned int m_id;
    static unsigned int s_lastId;

    HistoricWayPoint(const Vector3& _pos)
      : m_pos(_pos)
    {
      s_lastId++;
      m_id = s_lastId;
    }
};

//*****************************************************************************
// Class InsertionSquad
//*****************************************************************************

class InsertionSquad : public Unit
{
  protected:
    LList<HistoricWayPoint*> m_positionHistory; // A list of all the places the user has clicked. Most recent first
    Vector3 m_lastTarget;

  public:
    int m_weaponType; // Indexes into GlobalResearch
    int m_controllerId; // Task ID of controller if this squad is running one
    int m_teleportId; // Id of teleport build we wish to enter, or -1

    Vector3 m_focusPos;
    int m_numControlled;
    int m_numControlledThisFrame;

    InsertionSquad(int teamId, int _unitId, int numEntities, const Vector3& _pos);
    ~InsertionSquad() override;

    bool Advance(int _slice) override;

    void RunAI(AI* _ai) override;

    void SetWayPoint(const Vector3& _pos) override;
    Vector3 GetTargetPos(double _distFromPointMan);
    Entity* GetPointMan();
    void SetWeaponType(int _weaponType); // Indexes into GlobalResearch
    void CycleSecondary();
    void Attack(Vector3 pos, bool withGrenade) override;

    void DirectControl(const TeamControls& _teamControls) override;
    // used when the squad is being directly controlled by the player using a control pad

    bool IsSelectable() override;
};

//*****************************************************************************
// Class Squadie
//*****************************************************************************

class Squadie : public Entity
{
  public:
    bool m_justFired;
    double m_secondaryTimer;

  protected:
    ShapeMarker* m_laser;
    ShapeMarker* m_brass;

    WorldObjectId m_enemyId;
    double m_retargetTimer;
    void RunAI(); // Call this if the player isnt' controlling us

  public:
    Squadie();

    void Begin() override;
    bool Advance(Unit* _unit) override;
    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;

    void Attack(const Vector3& _pos) override;

    void Render(double _predictionTime) override;

    bool HasSecondaryWeapon();
    void FireSecondaryWeapon(const Vector3& _pos);

    Vector3 GetCameraFocusPoint() override;

    Vector3 GetSecondaryWeaponTarget();
    Vector3 GetSecondaryWeaponTarget(TeamControls _teamControls);
};

#endif
