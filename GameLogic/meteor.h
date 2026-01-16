#ifndef _included_meteor_h
#define _included_meteor_h

#include "entity.h"

class Meteor : public Entity
{
  public:
    Vector3 m_targetPos;
    double m_life;
    bool m_timeSlowTriggered;

    Meteor();
    ~Meteor() override;

    void Begin() override;

    bool Advance(Unit* _unit) override;
    bool AdvanceToTarget();

    void Render(double _predictionTime) override;
};

#endif
