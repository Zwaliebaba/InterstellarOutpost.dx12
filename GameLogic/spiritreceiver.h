#ifndef _included_spiritreceiver_h
#define _included_spiritreceiver_h

#include "building.h"

class SpiritProcessor;
class UnprocessedSpirit;

// ****************************************************************************
// Class ReceiverBuilding
// ****************************************************************************

class ReceiverBuilding : public Building
{
  protected:
    int m_spiritLink;
    ShapeMarker* m_spiritLocation;

    LList<double> m_spirits;

  public:
    ReceiverBuilding();

    void Initialise(Building* _template) override;
    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    bool IsInView() override;
    virtual Vector3 GetSpiritLocation();
    virtual void TriggerSpirit(double _initValue);

    static SpiritProcessor* GetSpiritProcessor();

    static void BeginRenderUnprocessedSpirits();
    static void RenderUnprocessedSpirit(const Vector3& _pos, double _life = 1.0); // gl friendly
    static void RenderUnprocessedSpirit_basic(const Vector3& _pos, double _life = 1.0); // dx friendly
    static void RenderUnprocessedSpirit_detail(const Vector3& _pos, double _life = 1.0); // dx friendly
    static void EndRenderUnprocessedSpirits();

    void Read(TextReader* _in, bool _dynamic) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;
};

// ****************************************************************************
// Class SpiritProcessor
// ****************************************************************************

class SpiritProcessor : public ReceiverBuilding
{
  protected:
    double m_timerSync;
    int m_numThisSecond;
    double m_spawnSync;
    double m_throughput;

  public:
    LList<UnprocessedSpirit*> m_floatingSpirits;

    SpiritProcessor();

    void TriggerSpirit(double _initValue) override;

    void GetObjectiveCounter(UnicodeString& _dest) override;

    void Initialise(Building* _building) override;
    bool Advance() override;
    bool IsInView() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
};

// ****************************************************************************
// Class ReceiverLink
// ****************************************************************************

class ReceiverLink : public ReceiverBuilding
{
  public:
    ReceiverLink();
    bool Advance() override;
};

// ****************************************************************************
// Class ReceiverSpiritSpawner
// ****************************************************************************

class ReceiverSpiritSpawner : public ReceiverBuilding
{
  public:
    ReceiverSpiritSpawner();
    bool Advance() override;
};

// ****************************************************************************
// Class SpiritReceiver
// ****************************************************************************

#define SPIRITRECEIVER_NUMSTATUSMARKERS 5

class SpiritReceiver : public ReceiverBuilding
{
  protected:
    ShapeMarker* m_headMarker;
    Shape* m_headShape;
    ShapeMarker* m_spiritLink;
    ShapeMarker* m_statusMarkers[SPIRITRECEIVER_NUMSTATUSMARKERS];

  public:
    SpiritReceiver();

    Vector3 GetSpiritLocation() override;

    void Initialise(Building* _template) override;
    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderPorts() override;

};

// ****************************************************************************
// Class UnprocessedSpirit
// ****************************************************************************

class UnprocessedSpirit : public WorldObject
{
  protected:
    double m_timeSync;
    double m_positionOffset; // Used to make them double around a bit
    double m_xaxisRate;
    double m_yaxisRate;
    double m_zaxisRate;

  public:
    Vector3 m_hover;

    enum
    {
      StateUnprocessedFalling,
      StateUnprocessedFloating,
      StateUnprocessedDeath
    };

    int m_state;

    UnprocessedSpirit();

    bool Advance() override;
    double GetLife(); // Returns 0.0-1.0 (0.0=dead, 1.0=alive)

    void Render(double _predictionTime) override;
};

#endif
