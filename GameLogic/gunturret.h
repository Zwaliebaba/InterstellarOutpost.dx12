#ifndef _included_gunturret_h
#define _included_gunturret_h

#include "building.h"

#define GUNTURRET_RETARGETTIMER     3.0
#define GUNTURRET_MULTIWINIA_RETARGETTIMER 1.0
#define GUNTURRET_NUMSTATUSMARKERS  5
#define GUNTURRET_NUMBARRELS        4
#define GUNTURRET_OWNERSHIPTIMER    1.0

#define GUNTURRET_MINRANGE          100.0
#define GUNTURRET_MAXRANGE          300.0
#define FLAMETURRET_MINRANGE        10.0
#define FLAMETURRET_MAXRANGE        150.0
#define ROCKETTURRET_MINRANGE       100.0
#define ROCKETTURRET_MAXRANGE       200.0
#define SUBVERSIONTURRET_MINRANGE   10.0f
#define SUBVERSIONTURRET_MAXRANGE   150.0f

class AITarget;

class GunTurret : public Building
{
  protected:
    Shape* m_turret;
    Shape* m_barrel;
    ShapeMarker* m_barrelMount;
    ShapeMarker* m_barrelEnd[GUNTURRET_NUMBARRELS];
    ShapeMarker* m_shellEject[GUNTURRET_NUMBARRELS];
    ShapeMarker* m_statusMarkers[GUNTURRET_NUMSTATUSMARKERS];
    ShapeMarker* m_scoreMarker;

    Vector3 m_turretFront;
    Vector3 m_barrelUp;

    bool m_aiTargetCreated;

    Vector3 m_target;
    WorldObjectId m_targetId;
    int m_nextBarrel;
    double m_retargetTimer;
    bool m_targetCreated;

    double m_health;
    double m_ownershipTimer;
    double m_hadTargetTimer;

    int m_kills[NUM_TEAMS];
    double m_temperature;
    double m_overheatTimer;

  public:
    int m_state;
    AITarget* m_aiTarget;

    // used for the grenade turret
    Vector3 m_targetPos;
    double m_targetRadius;
    double m_targetForce;

    bool m_friendlyFireWarning; // True if firing = lots of friendly deaths
    double m_fireTimer;

    Vector3 m_actualTargetPos; // Where our barrels are actually aiming

    enum
    {
      GunTurretTypeStandard = 0,
      GunTurretTypeRocket,
      GunTurretTypeMortar,
      GunTurretTypeFlame,
      GunTurretTypeSubversion,
      NumGunTurretTypes
    };

  protected:
    bool SearchForTargets();
    void SearchForRandomPos();
    void RecalculateOwnership();

    bool ClearLOS(const Vector3& _pos);

  public:
    GunTurret();

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    void ExplodeBody();
    void Damage(double _damage) override;
    bool Advance() override;

    Vector3 GetTarget();

    bool IsInView() override;
    void Render(double _predictionTime) override;
    void RenderPorts() override;

    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen, Vector3* _pos, Vector3* norm) override;

    void SetTarget(Vector3 _target);
    void PrimaryFire();

    double GetReloadTime();

    void RegisterKill(int killedTeamId, int numKills);

    static char* GetTurretTypeName(int _type);
    static char* GetTurretModelName(int _type);

    static void SetupTeamShapes();

    double GetMinRange();
    double GetMaxRange();

    void Read(TextReader* _in, bool _dynamic) override;
};

class GunTurretTarget : public WorldObject
{
  public:
    int m_buildingId;

    GunTurretTarget(int _buildingId);
    bool Advance() override;
};

#endif
