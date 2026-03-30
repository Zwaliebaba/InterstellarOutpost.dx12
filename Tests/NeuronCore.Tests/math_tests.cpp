#include <CppUnitTest.h>
#include "vector3.h"
#include "matrix34.h"
#include "plane.h"
#include "float_vector3.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTests
{
  TEST_CLASS(Vector3Tests)
  {
  public:
    TEST_METHOD(Add_ReturnsComponentwiseSum)
    {
      vector3 a(1, 2, 3);
      vector3 b(4, 5, 6);
      vector3 result = a + b;
      Assert::AreEqual(5, result.x);
      Assert::AreEqual(7, result.y);
      Assert::AreEqual(9, result.z);
    }

    TEST_METHOD(Subtract_ReturnsComponentwiseDifference)
    {
      vector3 a(5, 7, 9);
      vector3 b(1, 2, 3);
      vector3 result = a - b;
      Assert::AreEqual(4, result.x);
      Assert::AreEqual(5, result.y);
      Assert::AreEqual(6, result.z);
    }

    TEST_METHOD(Magnitude_ReturnsCorrectLength)
    {
      vector3 v(3, 0, 4);
      Assert::AreEqual(5.0f, v.Magnitude(), 1e-5f);
    }

    TEST_METHOD(Normalise_ProducesUnitVector)
    {
      vector3 v(0, 3, 4);
      v.Normalise();
      Assert::AreEqual(1.0f, v.Magnitude(), 1e-5f);
    }

    TEST_METHOD(DotProduct_ReturnsCorrectScalar)
    {
      vector3 a(1, 0, 0);
      vector3 b(0, 1, 0);
      Assert::AreEqual(0, a * b);
    }

    TEST_METHOD(CrossProduct_IsPerpendicularToBothInputs)
    {
      vector3 a(1, 0, 0);
      vector3 b(0, 1, 0);
      vector3 cross = a ^ b;
      Assert::AreEqual(0, cross * a, L"Cross product must be perpendicular to a");
      Assert::AreEqual(0, cross * b, L"Cross product must be perpendicular to b");
    }
  };

  TEST_CLASS(PlaneTests)
  {
  public:
    TEST_METHOD(DistanceToPoint_ReturnsZero_ForPointOnPlane)
    {
      // Plane with upward normal at y=0
      plane p;
      p.SetFromPointNormal(vector3(0, 0, 0), vector3(0, 1, 0));
      float dist = p.DistanceToPoint(vector3(5, 0, 3));
      Assert::AreEqual(0.0f, dist, 1e-5f);
    }

    TEST_METHOD(DistanceToPoint_ReturnsPositive_ForPointAbovePlane)
    {
      plane p;
      p.SetFromPointNormal(vector3(0, 0, 0), vector3(0, 1, 0));
      float dist = p.DistanceToPoint(vector3(0, 5, 0));
      Assert::IsTrue(dist > 0.0f);
    }
  };
}
