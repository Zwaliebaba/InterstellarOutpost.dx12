#ifndef _included_laserfence_h
#define _included_laserfence_h

#include "building.h"

class TextReader;

#define LASERFENCE_RAISESPEED       0.3

class LaserFence : public Building
{
  protected:
    double m_status; // 0=down, 1=up    
    int m_nextLaserFenceId;
    double m_sparkTimer;

    bool m_radiusSet;

    ShapeMarker* m_marker1;
    ShapeMarker* m_marker2;

    bool m_nextToggled; // set to true when the fence has enabled/disabled the next fence in the line, to prevent constant enable calling
    bool m_solarMode; // receiving power from solar panels, so switch off if the supply stops
    bool m_solarLinked; // a laser fence that is connected to us is solar powered, which means if it shuts down, we will too

    double m_surgeReceived;

  public:
    enum
    {
      ModeDisabled,
      ModeEnabling,
      ModeEnabled,
      ModeDisabling,
      ModeNeverOn,
      ModeAlwaysOn
    };

    int m_mode;
    double m_scale;

    LaserFence();

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    bool Advance() override;
    void Render(double predictionTime) override;
    void RenderAlphas(double predictionTime) override;
    void RenderLights() override;

    bool PerformDepthSort(Vector3& _centrePos) override;
    bool IsInView() override;

    void Read(TextReader* _in, bool _dynamic) override;
    void Write(TextWriter* _out) override;

    void Enable();
    void Disable();
    void Toggle();
    bool IsEnabled();

    void Spark();
    void Electrocute(const Vector3& _pos);

    void SetSolarLinked();

    void ProvidePower();

    int GetBuildingLink() override;
    void SetBuildingLink(int _buildingId) override;

    double GetFenceFullHeight();

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;

    void ListSoundEvents(LList<char*>* _list) override;

    Vector3 GetTopPosition();
};

#endif
