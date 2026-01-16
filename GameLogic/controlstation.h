#ifndef _included_controlstation_h
#define _included_controlstation_h

#include "building.h"

class ControlStation : public Building
{
  protected:
    int m_controlBuildingId;
    int m_pulseBombId;

  public:
    ControlStation();
    bool Advance() override;

    void Initialise(Building* _template) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    void RecalculateOwnership();

    void RenderPorts() override;
    void RenderAlphas(double _predictionTime) override;
    bool PerformDepthSort(Vector3& _centrePos) override;

    void LinkToPulseBomb(int buildingId);

    bool IsInView() override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
