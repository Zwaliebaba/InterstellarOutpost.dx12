#ifndef _included_airstrike_h
#define _included_airstrike_h

#include "unit.h"

class AirstrikeUnit : public Unit
{
  public:
    Vector3 m_enterPosition;
    Vector3 m_attackPosition;
    Vector3 m_exitPosition;

    Vector3 m_front; // Current direction
    Vector3 m_up;
    double m_speed;

    bool m_napalm;

    int m_effectId;
    int m_numInvaders;

    enum
    {
      StateApproaching,
      StateLeaving
    };

    int m_state;

    bool AdvanceToTargetPosition(Vector3 _targetPos);

    AirstrikeUnit(int teamId, int unitId, int numEntities, const Vector3& _pos);

    void Begin() override;
    bool Advance(int _slice) override;
    void Render(double _predictionTime) override;

    bool IsInView() override;
};

class SpaceInvader : public Entity
{
  protected:
    Vector3 m_targetPos;
    bool m_armed;
    Shape* m_bombShape;

  public:
    SpaceInvader();

    bool Advance(Unit* _unit) override;
    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;
    void Render(double _predictionTime) override;
};

#endif
