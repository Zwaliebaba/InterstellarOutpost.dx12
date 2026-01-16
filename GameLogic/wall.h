#ifndef _included_wall_h
#define _included_wall_h

#include "building.h"

class Wall : public Building
{
  protected:
    double m_damage;
    double m_fallSpeed;
    double m_lastDamageTime;
    double m_scale;

    int m_objectiveLink;
    bool m_registered;

  public:
    Wall();
    void Initialise(Building* _template) override;

    bool Advance() override;
    void Damage(double _damage) override;
    void Render(double _predictionTime) override;

    void SetDetail(int _detail) override;

    void SetBuildingLink(int _buildingId) override;
    int GetBuildingLink() override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

    void Read(TextReader* _in, bool _dynamic) override;
};

#endif
