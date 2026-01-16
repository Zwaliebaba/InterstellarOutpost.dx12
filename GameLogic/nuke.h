#ifndef _included_nuke_h
#define _included_nuke_h

#include "entity.h"

class Nuke : public Entity
{
  protected:
    double m_totalDistance;

    LList<Vector3*> m_history;
    double m_historyTimer;

    bool m_exploded;
    bool m_launched;
    bool m_armed;

    double m_casualtyMessageTimer;
    int m_numDeaths;

    Vector3 m_subPos;
    Vector3 m_subVel;
    double m_subTimer;

    double m_launchTimer;

  public:
    Vector3 m_targetPos;
    bool m_renderMarker;

  protected:
    void RenderHistory(double _predictionTime);
    void RenderSub(double _predictionTime);
    void RenderGroundMarker();

  public:
    Nuke();
    ~Nuke() override;

    void Begin() override;

    bool Advance(Unit* _unit) override;
    bool AdvanceToTarget();
    bool AdvanceLaunched();
    bool AdvanceSub();

    void Render(double _predictionTime) override;

    void Launch();
};

#endif
