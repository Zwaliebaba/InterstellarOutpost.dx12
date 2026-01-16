#ifndef _included_jumppad_h
#define _included_jumppad_h

#include "building.h"

class JumpPad : public Building
{
  public:
    double m_force;
    double m_angle;

  protected:
    double m_launchTimer;

  public:
    JumpPad();
    void Initialise(Building* _template) override;

    bool Advance() override;
    void Render(double _predictionTime) override;
    void RenderAlphas(double _predictionTime) override;

    void Read(TextReader* _in, bool _dynamic) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;
};

#endif
