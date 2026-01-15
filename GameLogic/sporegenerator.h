#ifndef _included_sporegenerator_h
#define _included_sporegenerator_h

#include "entity.h"

#define SPOREGENERATOR_NUMTAILS 4

class SporeGenerator : public Entity
{
  public:
    double m_retargetTimer;
    double m_eggTimer;
    Vector3 m_targetPos;
    int m_spiritId;

  protected:
    bool SearchForRandomPos();
    bool SearchForSpirits();

    bool AdvanceToTargetPosition();
    bool AdvanceIdle();
    bool AdvanceEggLaying();
    bool AdvancePanic();

    void RenderTail(const Vector3& _from, const Vector3& _to, double _size);

    ShapeMarker* m_eggMarker;
    ShapeMarker* m_tail[SPOREGENERATOR_NUMTAILS];

    enum
    {
      StateIdle,
      StateEggLaying,
      StatePanic
    };

    int m_state;

  public:
    SporeGenerator();

    void Begin() override;
    bool Advance(Unit* _unit) override;
    bool ChangeHealth(int _amount, int _damageType) override;

    bool IsInView() override;
    void Render(double _predictionTime) override;

    void ListSoundEvents(LList<char*>* _list) override;
};

#endif
