#ifndef _included_trunkport_h
#define _included_trunkport_h

#include "building.h"

#define TRUNKPORT_HEIGHTMAP_MAXSIZE 16

class TrunkPort : public Building
{
  public:
    int m_targetLocationId;

    ShapeMarker* m_destination1;
    ShapeMarker* m_destination2;

    int m_heightMapSize;
    Vector3* m_heightMap;
    double m_openTimer;
    int m_populationLock;

    TrunkPort();
    ~TrunkPort() override;

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;
    bool Advance() override;
    void Render(double predictionTime) override;
    void RenderAlphas(double predictionTime) override;

    bool PerformDepthSort(Vector3& _centrePos) override;

    void ReprogramComplete() override;
    bool PopulationLocked();

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
