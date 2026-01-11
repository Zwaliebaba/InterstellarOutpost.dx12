#ifndef _included_armour_h
#define _included_armour_h

#include "entity.h"
#include "flag.h"

class AI;
class Building;

#define ARMOUR_UNLOADPERIOD   0.1

class Armour : public Entity
{

  protected:
    ShapeMarker* m_markerEntrance;
    Vector3 m_pickupPoint;
    double m_speed;

    int m_numPassengers;
    int m_registeredPassengers;
    int m_passengerOverflow;
    double m_previousUnloadTimer;
    double m_newOrdersTimer;

    Flag m_flag;

    int m_suicideBuildingId;
    int m_currentAIObjective;
    bool m_turretRushing;

    double m_recheckPathTimer;

    Vector3 m_smoothFront;
    Vector3 m_smoothUp;
    double m_smoothYpos;

  public:
    enum
    {
      StateIdle,
      StateUnloading,
      StateLoading,
    };

    Vector3 m_wayPoint;
    int m_state;
    Vector3 m_up;

    bool m_awaitingDeployment;
    Vector3 m_conversionPoint;
    int m_conversionOrder;

    static LList<int> s_armourObjectives;

  protected:
    bool PreventLoadOverlap();
    // make sure we arent loading while another armour nearby is already doing so; return true if we find an armour already loading
    void FindIdleDarwinians(); // scan for idle darwinians for loading into the rocket, and go pick them up
    void RRFindPickupPoint(); // look for somewhere to collect darwinians from (trunkport or spawn point)
    bool RRValidPickupPoint(const Vector3& _pos); // make sure there are no other armours on teh way to collect from this position
    void RRFindDropoffPoint(AI* _ai); // find somewhere to drop our darwinians (at objectives representing solar panels)
    void RunRocketRiotAI(AI* _ai);

    void RunAssaultAI(AI* _ai);
    void RunStandardAI(AI* _ai);
    void RunBlitzkriegAI(AI* _ai);
    void RunMobileBombAI(AI* _ai);
    // for whatever reason (Assault mode generally), there is nothing for the armour to do, so find a good target and go explode on it

    int CountApproachingDarwinians();
    void GetBlitzkriegDropPoint(Building* _b);

    bool IsRouteClear(const Vector3& _pos);
    void CreateRoute(const Vector3& _pos);

    void FindPickupPoint(AI* _ai);
    void AISetConversionPoint(const Vector3& _pos);

  public:
    Armour();
    ~Armour() override;

    void Begin() override;
    bool Advance(Unit* _unit) override;
    void Render(double _predictionTime) override;

    void RunAI(AI* _ai) override;

    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;

    void SetOrders(const Vector3& _orders);
    void SetWayPoint(const Vector3& _wayPoint);
    void SetConversionPoint(const Vector3& _conversionPoint);
    void AdvanceToTargetPos();
    void DetectCollisions();
    void ConvertToGunTurret();

    void SetDirectOrders(); // orders while in direct control mode - removes gun turret mode

    bool IsLoading();
    bool IsUnloading();
    void ToggleLoading();
    void AddPassenger();
    void RemovePassenger();

    void ToggleConversionAction();

    int Capacity();
    int GetNumPassengers();
    double GetSpeed();

    bool IsSelectable() override;

    void ListSoundEvents(LList<char*>* _list) override;

    void GetEntrance(Vector3& _exitPos, Vector3& _exitDir);

    void RegisterPassenger();
    void FollowRoute() override;

    bool RayHit(const Vector3& rayStart, const Vector3& rayDir) override;

    static bool IsRouteClear(const Vector3& _fromPos, const Vector3& _toPos);
};

#endif
