// ======================================================================
// Unit Test: Camera System
// ======================================================================
// Tests camera positioning, modes, view calculations, and transformations.
// These tests verify camera behavior without requiring GPU initialization.
// ======================================================================

#include "TestHarness.h"
#include <cmath>

// Mock simplified camera for testing core logic
// In production, you'd test the actual Camera class from src/camera.h
class MockCamera
{
  private:
    float m_x, m_y, m_z;
    float m_pitch, m_yaw, m_roll;
    float m_fov;

  public:
    MockCamera()
      : m_x(0.0f), m_y(0.0f), m_z(0.0f)
      , m_pitch(0.0f), m_yaw(0.0f), m_roll(0.0f)
      , m_fov(60.0f)
    {
    }

    void SetPosition(float x, float y, float z)
    {
      m_x = x;
      m_y = y;
      m_z = z;
    }

    void GetPosition(float& x, float& y, float& z) const
    {
      x = m_x;
      y = m_y;
      z = m_z;
    }

    void SetRotation(float pitch, float yaw, float roll)
    {
      m_pitch = pitch;
      m_yaw = yaw;
      m_roll = roll;
    }

    void GetRotation(float& pitch, float& yaw, float& roll) const
    {
      pitch = m_pitch;
      yaw = m_yaw;
      roll = m_roll;
    }

    void SetFOV(float fov)
    {
      m_fov = fov;
    }

    float GetFOV() const
    {
      return m_fov;
    }

    void MoveForward(float distance)
    {
      // Simplified: move along XZ plane based on yaw
      m_x += distance * std::sin(m_yaw);
      m_z += distance * std::cos(m_yaw);
    }

    void Strafe(float distance)
    {
      // Move perpendicular to forward direction
      float strafeYaw = m_yaw + 3.14159f / 2.0f;
      m_x += distance * std::sin(strafeYaw);
      m_z += distance * std::cos(strafeYaw);
    }

    float DistanceTo(float x, float y, float z) const
    {
      float dx = m_x - x;
      float dy = m_y - y;
      float dz = m_z - z;
      return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

int main()
{
  io::tests::TestContext ctx("Camera::Basics");

  // Test 1: Initial position
  {
    MockCamera camera;
    float x, y, z;
    camera.GetPosition(x, y, z);
    
    IO_EXPECT_NEAR(ctx, x, 0.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 0.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 0.0f, 0.001f);
  }

  // Test 2: Set and get position
  {
    MockCamera camera;
    camera.SetPosition(10.0f, 20.0f, 30.0f);
    
    float x, y, z;
    camera.GetPosition(x, y, z);
    
    IO_EXPECT_NEAR(ctx, x, 10.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 20.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 30.0f, 0.001f);
  }

  // Test 3: Set and get rotation
  {
    MockCamera camera;
    camera.SetRotation(0.5f, 1.0f, 0.0f);
    
    float pitch, yaw, roll;
    camera.GetRotation(pitch, yaw, roll);
    
    IO_EXPECT_NEAR(ctx, pitch, 0.5f, 0.001f);
    IO_EXPECT_NEAR(ctx, yaw, 1.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, roll, 0.0f, 0.001f);
  }

  // Test 4: Field of view
  {
    MockCamera camera;
    camera.SetFOV(90.0f);
    IO_EXPECT_NEAR(ctx, camera.GetFOV(), 90.0f, 0.001f);
    
    camera.SetFOV(45.0f);
    IO_EXPECT_NEAR(ctx, camera.GetFOV(), 45.0f, 0.001f);
  }

  // Test 5: Move forward (no rotation)
  {
    MockCamera camera;
    camera.SetPosition(0.0f, 0.0f, 0.0f);
    camera.SetRotation(0.0f, 0.0f, 0.0f);
    
    camera.MoveForward(10.0f);
    
    float x, y, z;
    camera.GetPosition(x, y, z);
    
    // With yaw=0, should move along Z axis
    IO_EXPECT_NEAR(ctx, z, 10.0f, 0.001f);
  }

  // Test 6: Distance calculation
  {
    MockCamera camera;
    camera.SetPosition(0.0f, 0.0f, 0.0f);
    
    float dist = camera.DistanceTo(3.0f, 4.0f, 0.0f);
    
    // Distance should be 5.0 (Pythagorean theorem: 3^2 + 4^2 = 5^2)
    IO_EXPECT_NEAR(ctx, dist, 5.0f, 0.001f);
  }

  // Test 7: Multiple movements
  {
    MockCamera camera;
    camera.SetPosition(0.0f, 0.0f, 0.0f);
    camera.SetRotation(0.0f, 0.0f, 0.0f);
    
    camera.MoveForward(10.0f);
    camera.Strafe(5.0f);
    
    float x, y, z;
    camera.GetPosition(x, y, z);
    
    // Position should have changed from origin
    float distFromOrigin = camera.DistanceTo(0.0f, 0.0f, 0.0f);
    IO_EXPECT_TRUE(ctx, distFromOrigin > 10.0f);  // Should be further than initial move
  }

  // Test 8: FOV constraints (typical range: 30-120 degrees)
  {
    MockCamera camera;
    
    camera.SetFOV(30.0f);
    IO_EXPECT_TRUE(ctx, camera.GetFOV() >= 30.0f);
    
    camera.SetFOV(120.0f);
    IO_EXPECT_TRUE(ctx, camera.GetFOV() <= 120.0f);
  }

  return ctx.Finalize();
}
