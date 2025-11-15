// ======================================================================
// Unit Test: NeuronClient DirectX Math Utilities
// ======================================================================
// Tests the OpenGL/DirectX math conversion and utility functions.
// These tests verify vector operations, transformations, and coordinate
// system conversions used throughout the rendering pipeline.
// ======================================================================

#include "TestHarness.h"

// Include DirectX Math headers from NeuronClient
#include <DirectXMath.h>

using namespace DirectX;

int main()
{
  io::tests::TestContext ctx("NeuronClient::DirectXMath");

  // Test 1: Vector creation and basic operations
  {
    XMVECTOR v1 = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
    XMVECTOR v2 = XMVectorSet(4.0f, 5.0f, 6.0f, 0.0f);
    
    XMVECTOR sum = XMVectorAdd(v1, v2);
    
    float x = XMVectorGetX(sum);
    float y = XMVectorGetY(sum);
    float z = XMVectorGetZ(sum);
    
    IO_EXPECT_NEAR(ctx, x, 5.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 7.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 9.0f, 0.001f);
  }

  // Test 2: Vector normalization
  {
    XMVECTOR v = XMVectorSet(3.0f, 4.0f, 0.0f, 0.0f);
    XMVECTOR normalized = XMVector3Normalize(v);
    
    float length = XMVectorGetX(XMVector3Length(normalized));
    IO_EXPECT_NEAR(ctx, length, 1.0f, 0.001f);
    
    float x = XMVectorGetX(normalized);
    float y = XMVectorGetY(normalized);
    IO_EXPECT_NEAR(ctx, x, 0.6f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 0.8f, 0.001f);
  }

  // Test 3: Dot product
  {
    XMVECTOR v1 = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR v2 = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    
    XMVECTOR dotResult = XMVector3Dot(v1, v2);
    float dot = XMVectorGetX(dotResult);
    
    // Perpendicular vectors have dot product of 0
    IO_EXPECT_NEAR(ctx, dot, 0.0f, 0.001f);
  }

  // Test 4: Cross product (right-hand rule)
  {
    XMVECTOR v1 = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);  // X-axis
    XMVECTOR v2 = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  // Y-axis
    
    XMVECTOR cross = XMVector3Cross(v1, v2);
    
    float x = XMVectorGetX(cross);
    float y = XMVectorGetY(cross);
    float z = XMVectorGetZ(cross);
    
    // X cross Y = Z (right-hand rule)
    IO_EXPECT_NEAR(ctx, x, 0.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 0.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 1.0f, 0.001f);
  }

  // Test 5: Matrix identity
  {
    XMMATRIX identity = XMMatrixIdentity();
    
    XMVECTOR v = XMVectorSet(1.0f, 2.0f, 3.0f, 1.0f);
    XMVECTOR transformed = XMVector4Transform(v, identity);
    
    // Identity transform should not change the vector
    float x = XMVectorGetX(transformed);
    float y = XMVectorGetY(transformed);
    float z = XMVectorGetZ(transformed);
    
    IO_EXPECT_NEAR(ctx, x, 1.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 2.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 3.0f, 0.001f);
  }

  // Test 6: Translation matrix
  {
    XMMATRIX translation = XMMatrixTranslation(5.0f, 10.0f, 15.0f);
    
    XMVECTOR point = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR transformed = XMVector4Transform(point, translation);
    
    float x = XMVectorGetX(transformed);
    float y = XMVectorGetY(transformed);
    float z = XMVectorGetZ(transformed);
    
    IO_EXPECT_NEAR(ctx, x, 5.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 10.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 15.0f, 0.001f);
  }

  // Test 7: Rotation matrix (90 degrees around Z-axis)
  {
    XMMATRIX rotation = XMMatrixRotationZ(XM_PIDIV2);  // 90 degrees
    
    XMVECTOR v = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);  // Point on X-axis
    XMVECTOR rotated = XMVector3Transform(v, rotation);
    
    float x = XMVectorGetX(rotated);
    float y = XMVectorGetY(rotated);
    float z = XMVectorGetZ(rotated);
    
    // After 90° rotation around Z, X-axis points toward Y-axis
    IO_EXPECT_NEAR(ctx, x, 0.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, y, 1.0f, 0.001f);
    IO_EXPECT_NEAR(ctx, z, 0.0f, 0.001f);
  }

  return ctx.Finalize();
}
