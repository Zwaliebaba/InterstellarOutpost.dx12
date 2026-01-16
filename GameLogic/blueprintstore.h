#ifndef _included_blueprintstore_h
#define _included_blueprintstore_h

#include "building.h"

class BlueprintBuilding : public Building
{
  public:
    int m_buildingLink;
    double m_infected;
    int m_segment;

  protected:
    ShapeMarker* m_marker;
    Vector3 m_vel;

  public:
    BlueprintBuilding();

    void Initialise(Building* _template) override;
    bool Advance() override;
    bool IsInView() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    virtual void SendBlueprint(int _segment, bool _infected);

    Matrix34 GetMarker(double _predictionTime);

    void Read(TextReader* _in, bool _dynamic) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;
};

#define BLUEPRINTSTORE_NUMSEGMENTS  4

class BlueprintStore : public BlueprintBuilding
{
  public:
    double m_segments[BLUEPRINTSTORE_NUMSEGMENTS];

    BlueprintStore();

    void GetDisplay(Vector3& _pos, Vector3& _right, Vector3& _up, double& _size);

    void Initialise(Building* _template) override;
    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
    void SendBlueprint(int _segment, bool _infected) override;

    void GetObjectiveCounter(UnicodeString& _dest) override;

    int GetNumInfected(); // Returns number of segments totally infected ie == 100.0
    int GetNumClean(); // Returns number of segments totally clean ie == 0.0
};

class BlueprintConsole : public BlueprintBuilding
{
  public:
    BlueprintConsole();

    void Initialise(Building* _template) override;

    void RecalculateOwnership();
    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderPorts() override;

    void Read(TextReader* _in, bool _dynamic) override;
};

class BlueprintRelay : public BlueprintBuilding
{
  public:
    double m_altitude;

    BlueprintRelay();

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    bool Advance() override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
