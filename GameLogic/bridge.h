#ifndef _included_bridge_h
#define _included_bridge_h

#include "teleport.h"

#define BRIDGE_TRANSPORTPERIOD          0.1
#define BRIDGE_TRANSPORTSPEED           50.0

class Bridge : public Teleport
{
  public:
    enum
    {
      BridgeTypeEnd,
      BridgeTypeTower,
      NumBridgeTypes
    };

    int m_bridgeType;
    int m_nextBridgeId;
    double m_status; // Construction status, 0=not started, 100=finished, < 0.0 = shutdown

  protected:
    Shape* m_shapes[NumBridgeTypes];
    ShapeMarker* m_signal;

    bool m_beingOperated;

  public:
    Bridge();

    void Initialise(Building* _template) override;
    void SetBridgeType(int _type);

    void Render(double predictionTime) override;
    void RenderAlphas(double predictionTime) override;
    bool Advance() override;

    bool GetAvailablePosition(Vector3& _pos, Vector3& _front); // Finds place for engineer
    void BeginOperation();
    void EndOperation();

    bool ReadyToSend() override;
    Vector3 GetStartPoint() override;
    Vector3 GetEndPoint() override;
    bool GetExit(Vector3& _pos, Vector3& _front) override;

    bool UpdateEntityInTransit(Entity* _entity) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
