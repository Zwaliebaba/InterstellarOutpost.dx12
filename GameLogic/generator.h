#ifndef _included_generator_h
#define _included_generator_h

#include "building.h"

struct PowerSurge
{
  int m_teamId;
  double m_percent;
};

// ****************************************************************************
// Class PowerBuilding
// ****************************************************************************

class PowerBuilding : public Building
{
  protected:
    int m_powerLink;
    ShapeMarker* m_powerLocation;

    LList<PowerSurge*> m_surges;

  public:
    PowerBuilding();
    ~PowerBuilding() override;

    void Initialise(Building* _template) override;
    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    bool IsInView() override;
    Vector3 GetPowerLocation();

    virtual void TriggerSurge(double _initValue, int teamId = 255);

    void Read(TextReader* _in, bool _dynamic) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;
};

// ****************************************************************************
// Class Generator
// ****************************************************************************

class Generator : public PowerBuilding
{
  protected:
    ShapeMarker* m_counter;

    double m_timerSync;
    int m_numThisSecond;
    bool m_enabled;

  public:
    double m_throughput;

    Generator();

    void TriggerSurge(double _initValue, int teamId = 255) override;

    void ReprogramComplete() override;

    void GetObjectiveCounter(UnicodeString& _dest) override;

    bool Advance() override;
    void Render(double _predictionTime) override;
};

// ****************************************************************************
// Class Pylon
// ****************************************************************************

class Pylon : public PowerBuilding
{
  public:
    Pylon();
    bool Advance() override;
};

// ****************************************************************************
// Class PylonStart
// ****************************************************************************

class PylonStart : public PowerBuilding
{
  public:
    int m_reqBuildingId;

    PylonStart();

    void Initialise(Building* _template) override;
    bool Advance() override;
    void RenderAlphas(double _predictionTime) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

// ****************************************************************************
// Class PylonEnd
// ****************************************************************************

class PylonEnd : public PowerBuilding
{
  public:
    PylonEnd();

    void TriggerSurge(double _initValue, int teamId = 255) override;
    void RenderAlphas(double _predictionTime) override;
};

// ****************************************************************************
// Class SolarPanel
// ****************************************************************************

#define SOLARPANEL_NUMGLOWS 4
#define SOLARPANEL_NUMSTATUSMARKERS 5

class SolarPanel : public PowerBuilding
{
  protected:
    ShapeMarker* m_glowMarker[SOLARPANEL_NUMGLOWS];
    ShapeMarker* m_statusMarkers[SOLARPANEL_NUMSTATUSMARKERS];

    bool m_operating;
    int m_startingTeam;

  public:
    SolarPanel();

    void Initialise(Building* _template) override;
    bool Advance() override;

    void RecalculateOwnership();

    void Render(double _predictionTime) override;
    void RenderPorts() override;
    void RenderAlphas(double _predictionTime) override;

    bool IsOperating();
};

// ****************************************************************************
// Class 
// ****************************************************************************

class PowerSplitter : public PowerBuilding
{
  public:
    LList<int> m_links;

    PowerSplitter();

    void Initialise(Building* _template) override;

    void TriggerSurge(double _initValue, int teamId = 255) override;

    void RenderAlphas(double _predictionTime) override;

    void SetBuildingLink(int _buildingId) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
