#ifndef _included_clonelab_h
#define _included_clonelab_h

#include "incubator.h"

class CloneLab : public Incubator
{
  public:
    AITarget* m_aiTarget;
    int m_startingSpirits;

    CloneLab();
    ~CloneLab() override;

    void Initialise(Building* _template) override;
    void InitialiseAITarget();

    bool Advance() override;

    void RenderPorts() override;

    void Read(TextReader* _in, bool _dynamic) override;

    void RecalculateOwnership();
};

#endif
