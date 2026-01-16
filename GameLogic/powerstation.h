#ifndef _included_powerstation_h
#define _included_powerstation_h

#include "building.h"

class Powerstation : public Building
{
  protected:
    int m_linkedBuildingId;

  public:
    Powerstation();

    void Initialise(Building* _template) override;

    bool Advance() override;
    void Render(double predictionTime) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
