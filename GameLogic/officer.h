#ifndef _included_officer_h
#define _included_officer_h

#include "entity.h"
#include "flag.h"

#include "darray.h"

#define OFFICER_ATTACKRANGE     10.0
#define OFFICER_ABSORBRANGE     10.0

class OfficerFormation
{
  public:
    int m_entityUniqueId;
    double m_timer;
};

class Officer : public Entity
{
  public:
    enum
    {
      StateIdle,
      StateToWaypoint,
      StateGivingOrders
    };

    enum
    {
      OrderNone,
      OrderPrepareGoto,
      OrderGoto,
      OrderFollow,
      NumOrderTypes
    };

    int m_state;
    Vector3 m_wayPoint;
    int m_wayPointTeleportId; // Id of teleport we wish to walk into

    Vector3 m_targetFront; // used by the ai in formation mode

    int m_shield;
    bool m_demoted;
    bool m_absorb;
    double m_absorbTimer;
    bool m_noFormations;

    int m_orders;
    Vector3 m_orderPosition; // Position in the world
    int m_ordersBuildingId; // Id of target building eg Teleport
    int m_orderRouteId;
    double m_lastOrdersSet;
    double m_lastOrderCreated;

    bool m_formation;
    DArray<OfficerFormation> m_formationEntities;
    bool m_formationAngleSet;

    ShapeMarker* m_flagMarker;
    Flag m_flag;

  protected:
    bool AdvanceIdle();
    bool AdvanceToWaypoint();
    bool AdvanceGivingOrders();
    bool AdvanceToTargetPosition();
    bool AdvanceToTargetPositionInFormation();
    bool SearchForRandomPosition();

    void Absorb();

    void RenderShield(double _predictionTime);
    void RenderSpirit(const Vector3& _pos);

  public:
    Officer();
    ~Officer() override;

    void Begin() override;
    void Render(double _predictionTime) override;
    bool RenderShape(double _predictionTime);
    void RenderFlag(double _predictionTime);

    bool Advance(Unit* _unit) override;

    void RunAI(AI* _ai) override;

    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;

    void SetWaypoint(const Vector3& _wayPoint);
    void SetOrders(const Vector3& _orders, bool directRoute = false);

    bool IsFormationToggle(const Vector3& mousePos);
    void SetFormation(const Vector3& targetPos);

    void SetNextMode();
    void SetPreviousMode();

    void CancelOrders();
    void SetFollowMode();

    bool IsSelectable() override;

    int GetFormationIndex(int _uniqueId);
    Vector3 GetFormationPosition(int _uniqueId);
    static Vector3 GetFormationPositionFromIndex(const Vector3& pos, int positionIndex, const Vector3& front, int numEntities);

    bool FormationFull();
    bool IsInFormationMode();
    bool IsInFormation(int _uniqueId);
    void RegisterWithFormation(int _uniqueId);

    bool IsThereATeleportClose(const Vector3& _orders);

    void CancelOrderSounds();

    void CalculateBoundingSphere(Vector3& centre, double& radius);
    char* LogState(char* _message = nullptr) override;
};

class OfficerOrders : public WorldObject
{
  public:
    Vector3 m_wayPoint;
    double m_arrivedTimer;

    OfficerOrders();

    bool Advance() override;
    void Render(double _time) override;
};

class MultiwiniaOfficerOrders : public WorldObject
{
  public:
    Vector3 m_wayPoint;
    int m_routeId;
    int m_routeWayPointId;
    double m_arrivedTimer;
    double m_dropArrowTimer;

    MultiwiniaOfficerOrders();

    bool Advance() override;
    void FollowRoute();
    void Render(double _time) override;
};

class OfficerOrderTrail : public WorldObject
{
  public:
    double m_birthTime;
    Vector3 m_front;
    Vector3 m_right;

    OfficerOrderTrail();

    bool Advance() override;
    void Render(double _time) override;
};

#endif
