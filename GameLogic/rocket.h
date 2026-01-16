#ifndef _included_rocket_h
#define _included_rocket_h

#include "building.h"

class AITarget;

class FuelBuilding : public Building
{
  protected:
    ShapeMarker* m_fuelMarker;
    static Shape* s_fuelPipe;

  public:
    int m_fuelLink;
    double m_currentLevel;

    FuelBuilding();

    void Initialise(Building* _template) override;

    virtual void ProvideFuel(double _level);

    Vector3 GetFuelPosition();

    FuelBuilding* GetLinkedBuilding();

    bool Advance() override;

    bool IsInView() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    void Read(TextReader* _in, bool _dynamic) override;

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    void Destroy(double _intensity) override;
};

class FuelGenerator : public FuelBuilding
{
  protected:
    Shape* m_pump;
    ShapeMarker* m_pumpTip;
    double m_pumpMovement;
    double m_previousPumpPos;

    Vector3 GetPumpPos();

  public:
    double m_surges;

    FuelGenerator();

    void ProvideSurge();

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    void GetObjectiveCounter(UnicodeString& _dest) override;
};

class FuelPipe : public FuelBuilding
{
  public:
    FuelPipe();

    bool Advance() override;
};

class FuelStation : public FuelBuilding
{
  protected:
    ShapeMarker* m_entrance;
    AITarget* m_aiTarget;

  public:
    FuelStation();

    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    Vector3 GetEntrance();

    bool Advance() override;
    bool IsLoading();
    bool BoardRocket(WorldObjectId id);

    bool PerformDepthSort(Vector3& _centrePos) override;
};

class EscapeRocket : public FuelBuilding
{
  protected:
    ShapeMarker* m_booster;
    ShapeMarker* m_window[3];
    double m_shadowTimer;
    double m_cameraShake;

    Vector3 m_vel;
    double m_attackTimer;

    bool m_attackWarning; // set to true when the under attack warning has been issued, false when damage = 0
    bool m_refueledWarning;
    bool m_countdownWarning;

  public:
    enum
    {
      StateRefueling,
      StateLoading,
      StateIgnition,
      StateReady,
      StateCountdown,
      StateExploding,
      StateFlight,
      NumStates
    };

    int m_state;
    double m_fuel;
    int m_pipeCount;
    int m_passengers;
    double m_countdown;
    double m_damage;
    float m_fuelBeforeExplosion;
    int m_spawnBuildingId;
    bool m_spawnCompleted;
    double m_refuelRate;

  protected:
    void Refuel();
    void SetupSpectacle();
    void SetupAttackers();

    void AdvanceRefueling();
    void AdvanceLoading();
    void AdvanceIgnition();
    void AdvanceReady();
    void AdvanceCountdown();
    void AdvanceFlight();
    void AdvanceExploding();

    void SetupSounds();

  public:
    EscapeRocket();
    ~EscapeRocket() override;

    void Initialise(Building* _template) override;
    bool Advance() override;

    void ProvideFuel(double _level) override;
    bool SafeToLaunch();
    bool BoardRocket(WorldObjectId _id);
    void Damage(double _damage) override;

    bool IsSpectacle();
    bool IsInView() override;

    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    void Read(TextReader* _in, bool _dynamic) override;

    void GetObjectiveCounter(UnicodeString& _dest) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override; // pos/norm will not always be available

    static int GetStateId(const char* _state);
};

void ConvertFragmentColours(ShapeFragment* _frag, RGBAColour _col);

#endif
