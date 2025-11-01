// ======================================================================
// Unit Test: Routing System
// ======================================================================
// Tests pathfinding waypoint and route management.
// Verifies route creation, waypoint addition, and nearest-point queries.
// ======================================================================

#include "TestHarness.h"
#include <vector>
#include <cmath>
#include <limits>

// Mock waypoint and route system (simplified from src/RoutingSystem.h)
struct Vec3
{
  float x, y, z;
  
  Vec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
    : x(_x), y(_y), z(_z)
  {
  }

  float DistanceTo(const Vec3& other) const
  {
    float dx = x - other.x;
    float dy = y - other.y;
    float dz = z - other.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
  }
};

class Waypoint
{
  private:
    Vec3 m_pos;

  public:
    explicit Waypoint(const Vec3& pos)
      : m_pos(pos)
    {
    }

    Vec3 GetPosition() const
    {
      return m_pos;
    }

    void SetPosition(const Vec3& pos)
    {
      m_pos = pos;
    }
};

class Route
{
  private:
    int m_id;
    std::vector<Waypoint> m_waypoints;

  public:
    explicit Route(int id)
      : m_id(id)
    {
    }

    int GetId() const
    {
      return m_id;
    }

    void AddWaypoint(const Vec3& pos)
    {
      m_waypoints.emplace_back(pos);
    }

    size_t GetWaypointCount() const
    {
      return m_waypoints.size();
    }

    const Waypoint& GetWaypoint(size_t index) const
    {
      return m_waypoints[index];
    }

    int FindNearestWaypoint(const Vec3& pos) const
    {
      if (m_waypoints.empty())
        return -1;

      float minDist = std::numeric_limits<float>::max();
      int nearestIdx = 0;

      for (size_t i = 0; i < m_waypoints.size(); ++i)
      {
        float dist = m_waypoints[i].GetPosition().DistanceTo(pos);
        if (dist < minDist)
        {
          minDist = dist;
          nearestIdx = static_cast<int>(i);
        }
      }

      return nearestIdx;
    }

    float GetRouteLength() const
    {
      if (m_waypoints.size() < 2)
        return 0.0f;

      float length = 0.0f;
      for (size_t i = 1; i < m_waypoints.size(); ++i)
      {
        length += m_waypoints[i].GetPosition().DistanceTo(m_waypoints[i - 1].GetPosition());
      }
      return length;
    }
};

int main()
{
  io::tests::TestContext ctx("RoutingSystem::Routes");

  // Test 1: Route creation
  {
    Route route(1);
    IO_EXPECT_TRUE(ctx, route.GetId() == 1);
    IO_EXPECT_TRUE(ctx, route.GetWaypointCount() == 0);
  }

  // Test 2: Add waypoints
  {
    Route route(1);
    route.AddWaypoint(Vec3(0.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(10.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(10.0f, 10.0f, 0.0f));
    
    IO_EXPECT_TRUE(ctx, route.GetWaypointCount() == 3);
  }

  // Test 3: Waypoint retrieval
  {
    Route route(1);
    route.AddWaypoint(Vec3(5.0f, 10.0f, 15.0f));
    
    const Waypoint& wp = route.GetWaypoint(0);
    Vec3 pos = wp.GetPosition();
    
    IO_EXPECT_NEAR(ctx, pos.x, 5.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, pos.y, 10.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, pos.z, 15.0f, 0.001f);
  }

  // Test 4: Find nearest waypoint
  {
    Route route(1);
    route.AddWaypoint(Vec3(0.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(10.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(20.0f, 0.0f, 0.0f));
    
    // Query point close to second waypoint
    Vec3 queryPos(9.0f, 0.0f, 0.0f);
    int nearestIdx = route.FindNearestWaypoint(queryPos);
    
    IO_EXPECT_TRUE(ctx, nearestIdx == 1);
  }

  // Test 5: Find nearest waypoint - multiple dimensions
  {
    Route route(1);
    route.AddWaypoint(Vec3(0.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(5.0f, 5.0f, 5.0f));
    route.AddWaypoint(Vec3(10.0f, 10.0f, 10.0f));
    
    // Query point in 3D space
    Vec3 queryPos(6.0f, 6.0f, 6.0f);
    int nearestIdx = route.FindNearestWaypoint(queryPos);
    
    // Should be closest to the middle waypoint
    IO_EXPECT_TRUE(ctx, nearestIdx == 1);
  }

  // Test 6: Route length calculation
  {
    Route route(1);
    route.AddWaypoint(Vec3(0.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(10.0f, 0.0f, 0.0f));
    route.AddWaypoint(Vec3(10.0f, 10.0f, 0.0f));
    
    float length = route.GetRouteLength();
    
    // Length should be 10 + 10 = 20
    IO_EXPECT_NEAR(ctx, length, 20.0f, 0.001f);
  }

  // Test 7: Empty route
  {
    Route route(1);
    
    int nearestIdx = route.FindNearestWaypoint(Vec3(0.0f, 0.0f, 0.0f));
    IO_EXPECT_TRUE(ctx, nearestIdx == -1);
    
    float length = route.GetRouteLength();
    IO_EXPECT_NEAR(ctx, length, 0.0f, 0.001f);
  }

  // Test 8: Single waypoint route
  {
    Route route(1);
    route.AddWaypoint(Vec3(5.0f, 5.0f, 5.0f));
    
    float length = route.GetRouteLength();
    IO_EXPECT_NEAR(ctx, length, 0.0f, 0.001f);
    
    int nearestIdx = route.FindNearestWaypoint(Vec3(0.0f, 0.0f, 0.0f));
    IO_EXPECT_TRUE(ctx, nearestIdx == 0);
  }

  // Test 9: Complex route with many waypoints
  {
    Route route(1);
    
    // Create a spiral path
    for (int i = 0; i < 10; ++i)
    {
      float angle = static_cast<float>(i) * 0.628f;  // ~36 degrees
      float radius = static_cast<float>(i) * 2.0f;
      route.AddWaypoint(Vec3(radius * std::cos(angle), 0.0f, radius * std::sin(angle)));
    }
    
    IO_EXPECT_TRUE(ctx, route.GetWaypointCount() == 10);
    
    float length = route.GetRouteLength();
    IO_EXPECT_TRUE(ctx, length > 0.0f);  // Should have positive length
  }

  return ctx.Finalize();
}
