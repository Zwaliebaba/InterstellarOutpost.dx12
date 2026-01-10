#ifndef _included_tank_h
#define _included_tank_h

#include "entity.h"

#define TANK_NUMFLOATERS    3
#define TANK_CAPACITY       40
#define TANK_UNLOADPERIOD   0.1

class Tank : public Entity
{
  protected:
    Shape* m_shapeTurret;
    ShapeMarker* m_markerTurret;
    ShapeMarker* m_markerEntrance;
    ShapeMarker* m_markerBarrelEnd;

    Vector3 m_up;
    Vector3 m_turretFront;
    Vector3 m_wayPoint;
    Vector3 m_attackTarget;
    Vector3 m_missileTarget;

    double m_speed;
    double m_attackTimer;

    bool m_missileFired;

    enum
    {
      StateCombat,
      StateUnloading,
      StateLoading,
    };

    int m_state;
    int m_numPassengers;
    double m_previousUnloadTimer;

    double m_mineTimer;

  public:
    Tank();
    ~Tank() override;

    void Begin() override;
    bool Advance(Unit* _unit) override;
    void Render(double _predictionTime) override;
    bool RenderPixelEffect(double _predictionTime) override;

    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;

    void SetWayPoint(const Vector3& _wayPoint);
    void SetAttackTarget(const Vector3& _attackTarget);

    void DropMine();

    void SetMissileTarget(const Vector3& _startRay, const Vector3& _rayDir);
    void SetMissileTarget(const Vector3& _target);
    Vector3 GetMissileTarget();
    void LaunchMissile();

    void PrimaryFire();

    bool IsLoading();
    bool IsUnloading();
    void AddPassenger();
    void RemovePassenger();

    void GetEntrance(Vector3& _exitPos, Vector3& _exitDir);

    bool IsSelectable() override;

    void RunAI(AI* _ai) override;
    void RunTankBattleAI(AI* _ai);
    void RunStandardAI(AI* _ai);

    double GetAttackTimer() { return m_mineTimer; }
};

#endif
