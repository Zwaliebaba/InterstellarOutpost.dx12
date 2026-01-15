#ifndef _included_souldestroyer_h
#define _included_souldestroyer_h

#include "entity.h"

#define SOULDESTROYER_MINSEARCHRANGE       200.0
#define SOULDESTROYER_MAXSEARCHRANGE       300.0
#define SOULDESTROYER_DAMAGERANGE          25.0
#define SOULDESTROYER_MAXSPIRITS           50

class SoulDestroyer : public Entity
{
  protected:
    Vector3 m_targetPos;
    Vector3 m_up;
    WorldObjectId m_targetEntity;
    LList<Vector3> m_positionHistory;
    FastDArray<double> m_spirits;

    double m_retargetTimer;
    double m_panic;

    static Shape* s_shapeHead[NUM_TEAMS];
    static Shape* s_shapeTail[NUM_TEAMS];
    static ShapeMarker* s_tailMarker[NUM_TEAMS];

    Vector3 m_spiritPosition[SOULDESTROYER_MAXSPIRITS];

  public:
    static int s_numSoulDestroyers;

  protected:
    bool SearchForRandomPosition();
    bool SearchForTargetEnemy();
    bool SearchForRetreatPosition();

    bool AdvanceToTargetPosition();
    void RecordHistoryPosition();
    bool GetTrailPosition(Vector3& _pos, Vector3& _vel);

    void RenderShapes(double _predictionTime);
    void RenderSpirit(const Vector3& _pos, double _alpha);

    void Panic(double _time);

  public:
    SoulDestroyer();
    ~SoulDestroyer() override;

    void Begin() override;
    bool Advance(Unit* _unit) override;
    bool ChangeHealth(int _amount, int _damageType = DamageTypeUnresistable) override;
    void Render(double _predictionTime) override;

    void Attack(const Vector3& _pos) override;

    void ListSoundEvents(LList<char*>* _list) override;

    void SetWaypoint(Vector3 _waypoint) override;
};

class Zombie : public WorldObject
{
  public:
    Vector3 m_front;
    Vector3 m_up;
    double m_life;

    Vector3 m_hover;
    double m_positionOffset; // Used to make them double around a bit
    double m_xaxisRate;
    double m_yaxisRate;
    double m_zaxisRate;

    Zombie();

    bool Advance() override;
    void Render(double _predictionTime) override;
};

#endif
