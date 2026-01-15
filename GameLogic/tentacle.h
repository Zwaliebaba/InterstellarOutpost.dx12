#ifndef _included_tentacle_h
#define _included_tentacle_h

#include "entity.h"
#include "globals.h"

class Tentacle : public Entity
{
  protected:
    enum
    {
      TentacleStateIdle,
      TentacleStateSlamGround,
      TentacleStateSwipeAcross,
    };

    int m_state;

    double m_size;
    WorldObjectId m_next; // Guy infront of me
    WorldObjectId m_prev; // Guy behind me

    Vector3 m_targetDirection;
    Vector3 m_direction;
    int m_portalId;

    LList<Vector3> m_rotationHistory;

    double m_idleTimer;

    Vector3 m_previousVel;
    int m_positionId;

    bool m_impactDetected;

    static Shape* s_shapeBody[NUM_TEAMS];
    static Shape* s_shapeHead[NUM_TEAMS];

    bool AdvanceIdle();
    bool AdvanceSlam();
    bool AdvanceSwipe();

    void HandleGroundCollision();

    void RecordHistoryRotation();
    bool GetTrailPosition(Vector3& _pos, Vector3& _vel, int _numSteps);

  public:
    Tentacle();

    void Begin() override;
    bool Advance(Unit* _unit) override;
    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;
    void Render(double _predictionTime) override;

    bool IsInView() override;

    void SetPortalId(int _id);

    void Attack(const Vector3& _pos) override;
};

#endif
