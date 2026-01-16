#ifndef _included_carryablebuilding_h
#define _included_carryablebuilding_h

#include "building.h"

class CarryableBuilding : public Building
{
  public:
    double m_carryRadius;
    double m_scale;
    int m_numLifters;
    int m_numLiftersThisFrame;
    int m_minLifters;
    int m_maxLifters;
    bool m_lifted;
    double m_recountTimer;

    Vector3 m_waypoint;
    int m_routeId;
    int m_routeWayPointId;

    double m_speedScale;

    Vector3 m_requestedWaypoint;
    int m_requestedRouteId;
    int m_numRequests;

    CarryableBuilding();

    void Initialise(Building* _template) override;
    void SetShape(Shape* _shape) override;

    bool Advance() override;
    bool IsInView() override;
    void Render(double _predictionTime) override;

    void FollowRoute();

    void CalculateOwnership();

    Vector3 GetCarryPosition(int _uniqueId);
    double GetCarryPercentage();

    int* CalculateCollisions(const Vector3& _pos, int& _numCollisions, double& _collisionFactor); // with other carryable buildings

    virtual void SetWaypoint(const Vector3& _waypoint, int routeId);

    virtual void HandleCollision(double _force);

    bool DoesSphereHit(const Vector3& _pos, double _radius) override;
    bool DoesShapeHit(Shape* _shape, Matrix34 _transform) override;
    bool DoesRayHit(const Vector3& _rayStart, const Vector3& _rayDir, double _rayLen = 1e10, Vector3* _pos = nullptr,
                    Vector3* _norm = nullptr) override;

    double GetSpeedScale();

    static bool IsCarryableBuilding(int _type);
};

#endif
