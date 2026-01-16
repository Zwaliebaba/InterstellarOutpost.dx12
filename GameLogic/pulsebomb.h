#ifndef _included_pulsebomb_h
#define _included_pulsebomb_h

#include "building.h"

class PulseBomb : public Building
{
  public:
    bool m_active;
    double m_scale;
    double m_detonateTime;
    double m_startTime;

    LList<int> m_links;

    bool m_oneMinuteWarning;

    static int s_pulseBombs[NUM_TEAMS];
    static double s_defaultTime;

    PulseBomb();
    ~PulseBomb() override;

    bool Advance() override;

    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;
    void RenderCountdown(double _predictionTime);

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    void Destroy();
    void ExplodeShape(double _fraction);

    void SetBuildingLink(int _buildingId) override;

    void Read(TextReader* _in, bool _dynamic) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

};

#endif
