#ifndef INCLUDED_ROUTING_SYSTEM_H
#define INCLUDED_ROUTING_SYSTEM_H

#include "llist.h"
#include "darray.h"
#include "vector3.h"

class Route;
class WayPoint;
class RouteNode;
struct RoutingTableEntry;

// ****************************************************************************
// Class RoutingSystem
// ****************************************************************************

class RoutingSystem
{
  public:
    DArray<RouteNode*> m_nodes;
    DArray<Route*> m_routes;

    bool m_walkableRoute;

    RoutingSystem();
    ~RoutingSystem();

    virtual void Initialise() = 0;
    void Render();
    void RenderNodes();

    int GetNearestNode(const Vector3& pos, bool requireWalkable);

    int GenerateRoute(const Vector3& fromPos, const Vector3& toPos, bool directRoute = false, bool permitClimbing = true,
                      bool noRounding = false); // returns routeID, or -1 for error
    void DeleteRoute(int id);

    Route* GetRoute(int id);

    void HighlightRoute(int routeId, int teamId);
    void SetRouteIntensity(int routeId, double intensity); // 0 - 1 for intensity
    void RenderRouteLine(const Vector3& from, const Vector3& to, double intensity); //
};

class LandRoutingSystem : public RoutingSystem
{
  public:
    LandRoutingSystem();
    void Initialise() override;
};

class SeaRoutingSystem : public RoutingSystem
{
  public:
    SeaRoutingSystem();
    void Initialise() override;
};

// ****************************************************************************
// Class WayPoint
// ****************************************************************************

class WayPoint
{
  public:
    enum
    {
      Type3DPos,
      TypeGroundPos,
      TypeBuilding
    };

    int m_type;
    Vector3 m_pos;
    int m_buildingId;

    WayPoint();
    WayPoint(int _type, const Vector3& _pos);
    ~WayPoint();

    Vector3 GetPos();
    void SetPos(const Vector3& _pos);
};

class RouteNode
{
  public:
    WayPoint m_waypoint;
    LList<RoutingTableEntry*> m_routeTable;
    bool m_checkCliffs;

    RouteNode();
    ~RouteNode();

    void AddRoute(int targetNodeId, int nextNodeId, double totalDistance);
    int GetRouteId(int targetNodeId);
};

struct RoutingTableEntry
{
  int m_targetNode; // You want to end up here
  int m_nextNode; // Go here first
  double m_totalDistance; // This is the total remaining distance
};

// ****************************************************************************
// Class Route
// ****************************************************************************

#define ROUTE_OK               1
#define ROUTEID_FAILURE        -1

class Route
{
  public:
    int m_id;
    LList<WayPoint*> m_wayPoints;

    int m_status;
    double m_alpha;

    Route(int id);
    ~Route();

    void AddWayPoint(const Vector3& _pos);
    void AddWayPoint(int _buildingId);
    WayPoint* GetWayPoint(int _id);

    int GetIdOfNearestWayPoint(const Vector3& _pos);
    int GetIdOfNearestEdge(const Vector3& _pos, double* _dist);

    double GetWalkingDistance();

    Vector3 GetEndPoint();

    void Render();
};

#endif
