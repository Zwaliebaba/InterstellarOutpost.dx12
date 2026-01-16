#ifndef _included_harvester_h
#define _included_harvester_h

#include "llist.h"
#include "entity.h"

class Grenade;
class AI;

class Harvester : public Entity
{
  public:
    enum
    {
      StateIdle,
      // Hover around aimlessly
      StateToWaypoint,
      // Travelling to user waypoint
      StateToSpirit,
      // Travelling to spirit
      StateToIncubator,
      // Travelling to factory with spirit
      StateToSpawnPoint,
      StateHarvesting // Sucking up souls
    };

    int m_state;
    Vector3 m_wayPoint; // User specified waypoint

    bool m_vunerable; // can only take damage when this is true

  protected:
    double m_hoverHeight;

    bool m_movedThisAdvance;
    bool m_fieldFound; // Has floated over a soul field on the way to its target use this to make 
    // sure it stops to collect instead of moving the average position constantly

    Vector3 m_targetPos; // Our internal target position
    Vector3 m_targetFront; // and orientation

    double m_retargetTimer;
    LList<int> m_spirits; // Collector only, Spirits already collected
    LList<int> m_grenades; // List of sucked up grenades

    int m_positionId; // Position on the building we are working on

    LList<Vector3*> m_positionHistory;

    bool SearchForRandomPosition();
    bool SearchForSpirits();
    bool SearchForIncubator();
    bool SearchForSpawnPoint();

    bool AdvanceIdle();
    bool AdvanceToWaypoint();
    bool AdvanceToSpirit();
    bool AdvanceToIncubator();
    bool AdvanceToSpawnPoint();
    bool AdvanceHarvesting();
    bool AdvanceToTargetPos(); // returns have-I-Arrived?

    void AdvanceSpirits();
    void AdvanceGrenades();
    void AdvanceEngines();

    double GetHoverHeight();

  public:
    Harvester();
    ~Harvester() override;

    void Begin() override;
    bool Advance(Unit* _unit) override;

    void RunAI(AI* _ai) override;

    void SetWaypoint(const Vector3& _wayPoint);
    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;

    int GetNumSpirits();
    int GetMaxSpirits();
    void CollectSpirit(int _spiritId);
    void ReleaseSpirits();
    void ReleaseGrenades();

    Vector3 GetCollectorPos(); // Position of the soul collector, positioned above the harvester

    void Render(double predictionTime) override;
    void RenderShape(double predictionTime);
    void RenderBubble(double _predictionTime);
    void RenderBeam(double _predictionTime);

    char* GetCurrentAction();

    bool IsSelectable() override;
};

#endif
