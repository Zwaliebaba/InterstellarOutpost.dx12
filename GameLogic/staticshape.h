#ifndef _included_staticshape_h
#define _included_staticshape_h

#include "building.h"

class StaticShape : public Building
{
  public:
    char m_shapeName[256];
    double m_scale;

    StaticShape();

    void Initialise(Building* _template) override;
    void SetDetail(int _detail) override;

    void SetShapeName(char* _shapeName);
    void SetStringId(char* _stringId);

    bool Advance() override;
    void Render(double _predictionTime) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override; // pos/norm will not always be available

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
