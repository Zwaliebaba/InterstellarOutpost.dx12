#ifndef _included_goddish_h
#define _included_goddish_h

#include "building.h"

class GodDish : public Building
{
  public:
    bool m_activated;
    double m_timer;

    GodDish();

    void Initialise(Building* _template) override;

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    bool IsInView() override;

    void Activate();
    void DeActivate();
    void SpawnSpam(bool _isResearch);

    void TriggerSpam();
};

#endif
