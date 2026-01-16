#ifndef _included_restrictionzone_h
#define _included_restrictionzone_h

#include "building.h"

class RestrictionZone : public Building
{
  public:
    double m_size;
    int m_blockPowerups;

    static LList<int> s_restrictionZones;

    RestrictionZone();
    ~RestrictionZone() override;

    void Initialise(Building* _template) override;

    bool Advance() override;
    void RenderAlphas(double _predictionTime) override;

    static bool IsRestricted(Vector3 _pos, bool _powerups = false);

    void Read(TextReader* _in, bool _dynamic) override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;
};

#endif
