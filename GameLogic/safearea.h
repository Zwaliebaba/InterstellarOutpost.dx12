#ifndef _included_safearea_h
#define _included_safearea_h

#include "building.h"

class SafeArea : public Building
{
  public:
    double m_size;
    int m_entitiesRequired;
    int m_entityTypeRequired;

    double m_recountTimer;
    int m_entitiesCounted;

    SafeArea();

    void Initialise(Building* _template) override;
    bool Advance() override;

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

    void GetObjectiveCounter(UnicodeString& _dest) override;

    void Read(TextReader* _in, bool _dynamic) override;
    void Write(TextWriter* _out) override;
};

#endif
