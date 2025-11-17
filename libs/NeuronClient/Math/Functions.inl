
#pragma once

namespace Neuron::Math
{
  // To allow floats to implicitly construct Scalars, we need to clarify these operators and suppress
  // upconversion.
  inline bool operator<(Scalar lhs, float rhs)
  {
    return (float) lhs < rhs;
  }

  inline bool operator<=(Scalar lhs, float rhs)
  {
    return (float) lhs <= rhs;
  }

  inline bool operator>(Scalar lhs, float rhs)
  {
    return (float) lhs > rhs;
  }

  inline bool operator>=(Scalar lhs, float rhs)
  {
    return (float) lhs >= rhs;
  }

  inline bool operator==(Scalar lhs, float rhs)
  {
    return (float) lhs == rhs;
  }

  inline bool operator<(float lhs, Scalar rhs)
  {
    return lhs < (float) rhs;
  }

  inline bool operator<=(float lhs, Scalar rhs)
  {
    return lhs <= (float) rhs;
  }

  inline bool operator>(float lhs, Scalar rhs)
  {
    return lhs > (float) rhs;
  }

  inline bool operator>=(float lhs, Scalar rhs)
  {
    return lhs >= (float) rhs;
  }

  inline bool operator==(float lhs, Scalar rhs)
  {
    return lhs == (float) rhs;
  }

#define CREATE_SIMD_FUNCTIONS(TYPE)                                                                                                                  \
  inline TYPE Sqrt(TYPE s)                                                                                                                           \
  {                                                                                                                                                  \
    return TYPE(XMVectorSqrt(s));                                                                                                                    \
  }                                                                                                                                                  \
  inline TYPE Recip(TYPE s)                                                                                                                          \
  {                                                                                                                                                  \
    return TYPE(XMVectorReciprocal(s));                                                                                                              \
  }                                                                                                                                                  \
  inline TYPE RecipSqrt(TYPE s)                                                                                                                      \
  {                                                                                                                                                  \
    return TYPE(XMVectorReciprocalSqrt(s));                                                                                                          \
  }                                                                                                                                                  \
  inline TYPE Floor(TYPE s)                                                                                                                          \
  {                                                                                                                                                  \
    return TYPE(XMVectorFloor(s));                                                                                                                   \
  }                                                                                                                                                  \
  inline TYPE Ceiling(TYPE s)                                                                                                                        \
  {                                                                                                                                                  \
    return TYPE(XMVectorCeiling(s));                                                                                                                 \
  }                                                                                                                                                  \
  inline TYPE Round(TYPE s)                                                                                                                          \
  {                                                                                                                                                  \
    return TYPE(XMVectorRound(s));                                                                                                                   \
  }                                                                                                                                                  \
  inline TYPE Abs(TYPE s)                                                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorAbs(s));                                                                                                                     \
  }                                                                                                                                                  \
  inline TYPE Exp(TYPE s)                                                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorExp(s));                                                                                                                     \
  }                                                                                                                                                  \
  inline TYPE Pow(TYPE b, TYPE e)                                                                                                                    \
  {                                                                                                                                                  \
    return TYPE(XMVectorPow(b, e));                                                                                                                  \
  }                                                                                                                                                  \
  inline TYPE Log(TYPE s)                                                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorLog(s));                                                                                                                     \
  }                                                                                                                                                  \
  inline TYPE Sin(TYPE s)                                                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorSin(s));                                                                                                                     \
  }                                                                                                                                                  \
  inline TYPE Cos(TYPE s)                                                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorCos(s));                                                                                                                     \
  }                                                                                                                                                  \
  inline TYPE Tan(TYPE s)                                                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorTan(s));                                                                                                                     \
  }                                                                                                                                                  \
  inline TYPE ASin(TYPE s)                                                                                                                           \
  {                                                                                                                                                  \
    return TYPE(XMVectorASin(s));                                                                                                                    \
  }                                                                                                                                                  \
  inline TYPE ACos(TYPE s)                                                                                                                           \
  {                                                                                                                                                  \
    return TYPE(XMVectorACos(s));                                                                                                                    \
  }                                                                                                                                                  \
  inline TYPE ATan(TYPE s)                                                                                                                           \
  {                                                                                                                                                  \
    return TYPE(XMVectorATan(s));                                                                                                                    \
  }                                                                                                                                                  \
  inline TYPE ATan2(TYPE y, TYPE x)                                                                                                                  \
  {                                                                                                                                                  \
    return TYPE(XMVectorATan2(y, x));                                                                                                                \
  }                                                                                                                                                  \
  inline TYPE Lerp(TYPE a, TYPE b, TYPE t)                                                                                                           \
  {                                                                                                                                                  \
    return TYPE(XMVectorLerpV(a, b, t));                                                                                                             \
  }                                                                                                                                                  \
  inline TYPE Lerp(TYPE a, TYPE b, float t)                                                                                                          \
  {                                                                                                                                                  \
    return TYPE(XMVectorLerp(a, b, t));                                                                                                              \
  }                                                                                                                                                  \
  inline TYPE Max(TYPE a, TYPE b)                                                                                                                    \
  {                                                                                                                                                  \
    return TYPE(XMVectorMax(a, b));                                                                                                                  \
  }                                                                                                                                                  \
  inline TYPE Min(TYPE a, TYPE b)                                                                                                                    \
  {                                                                                                                                                  \
    return TYPE(XMVectorMin(a, b));                                                                                                                  \
  }                                                                                                                                                  \
  inline TYPE Clamp(TYPE v, TYPE a, TYPE b)                                                                                                          \
  {                                                                                                                                                  \
    return Min(Max(v, a), b);                                                                                                                        \
  }                                                                                                                                                  \
  inline BoolVector operator<(TYPE lhs, TYPE rhs)                                                                                                    \
  {                                                                                                                                                  \
    return XMVectorLess(lhs, rhs);                                                                                                                   \
  }                                                                                                                                                  \
  inline BoolVector operator<=(TYPE lhs, TYPE rhs)                                                                                                   \
  {                                                                                                                                                  \
    return XMVectorLessOrEqual(lhs, rhs);                                                                                                            \
  }                                                                                                                                                  \
  inline BoolVector operator>(TYPE lhs, TYPE rhs)                                                                                                    \
  {                                                                                                                                                  \
    return XMVectorGreater(lhs, rhs);                                                                                                                \
  }                                                                                                                                                  \
  inline BoolVector operator>=(TYPE lhs, TYPE rhs)                                                                                                   \
  {                                                                                                                                                  \
    return XMVectorGreaterOrEqual(lhs, rhs);                                                                                                         \
  }                                                                                                                                                  \
  inline BoolVector operator==(TYPE lhs, TYPE rhs)                                                                                                   \
  {                                                                                                                                                  \
    return XMVectorEqual(lhs, rhs);                                                                                                                  \
  }                                                                                                                                                  \
  inline TYPE Select(TYPE lhs, TYPE rhs, BoolVector mask)                                                                                            \
  {                                                                                                                                                  \
    return TYPE(XMVectorSelect(lhs, rhs, mask));                                                                                                     \
  }

  CREATE_SIMD_FUNCTIONS(Scalar)
  CREATE_SIMD_FUNCTIONS(Vector3)
  CREATE_SIMD_FUNCTIONS(Vector4)

#undef CREATE_SIMD_FUNCTIONS

  inline float Sqrt(float s)
  {
    return Sqrt(Scalar(s));
  }

  inline float Recip(float s)
  {
    return Recip(Scalar(s));
  }

  inline float RecipSqrt(float s)
  {
    return RecipSqrt(Scalar(s));
  }

  inline float Floor(float s)
  {
    return Floor(Scalar(s));
  }

  inline float Ceiling(float s)
  {
    return Ceiling(Scalar(s));
  }

  inline float Round(float s)
  {
    return Round(Scalar(s));
  }

  inline float Abs(float s)
  {
    return s < 0.0f ? -s : s;
  }

  inline float Exp(float s)
  {
    return Exp(Scalar(s));
  }

  inline float Pow(float b, float e)
  {
    return Pow(Scalar(b), Scalar(e));
  }

  inline float Log(float s)
  {
    return Log(Scalar(s));
  }

  inline float Sin(float s)
  {
    return Sin(Scalar(s));
  }

  inline float Cos(float s)
  {
    return Cos(Scalar(s));
  }

  inline float Tan(float s)
  {
    return Tan(Scalar(s));
  }

  inline float ASin(float s)
  {
    return ASin(Scalar(s));
  }

  inline float ACos(float s)
  {
    return ACos(Scalar(s));
  }

  inline float ATan(float s)
  {
    return ATan(Scalar(s));
  }

  inline float ATan2(float y, float x)
  {
    return ATan2(Scalar(y), Scalar(x));
  }

  inline float Lerp(float a, float b, float t)
  {
    return a + (b - a) * t;
  }

  inline float Max(float a, float b)
  {
    return a > b ? a : b;
  }

  inline float Min(float a, float b)
  {
    return a < b ? a : b;
  }

  inline float Clamp(float v, float a, float b)
  {
    return Min(Max(v, a), b);
  }

  inline Scalar Length(Vector3 v)
  {
    return Scalar(XMVector3Length(v));
  }

  inline Scalar LengthSquare(Vector3 v)
  {
    return Scalar(XMVector3LengthSq(v));
  }

  inline Scalar LengthRecip(Vector3 v)
  {
    return Scalar(XMVector3ReciprocalLength(v));
  }

  inline Scalar Dot(Vector3 v1, Vector3 v2)
  {
    return Scalar(XMVector3Dot(v1, v2));
  }

  inline Scalar Dot(Vector4 v1, Vector4 v2)
  {
    return Scalar(XMVector4Dot(v1, v2));
  }

  inline Vector3 Cross(Vector3 v1, Vector3 v2)
  {
    return Vector3(XMVector3Cross(v1, v2));
  }

  inline Vector3 Normalize(Vector3 v)
  {
    return Vector3(XMVector3Normalize(v));
  }

  inline Vector4 Normalize(Vector4 v)
  {
    return Vector4(XMVector4Normalize(v));
  }

  inline Matrix3 Transpose(const Matrix3 &mat)
  {
    return Matrix3(XMMatrixTranspose(mat));
  }

  inline Matrix3 InverseTranspose(const Matrix3 &mat)
  {
    const Vector3 x = mat.GetX();
    const Vector3 y = mat.GetY();
    const Vector3 z = mat.GetZ();

    const Vector3 inv0 = Cross(y, z);
    const Vector3 inv1 = Cross(z, x);
    const Vector3 inv2 = Cross(x, y);
    const Scalar rDet = Recip(Dot(z, inv2));

    // Return the adjoint / determinant
    return Matrix3(inv0, inv1, inv2) * rDet;
  }

  // inline Matrix3 Inverse( const Matrix3& mat ) { TBD }
  // inline Transform Inverse( const Transform& mat ) { TBD }

  // This specialized matrix invert assumes that the 3x3 matrix is orthogonal (and normalized).
  inline AffineTransform OrthoInvert(const AffineTransform &xform)
  {
    Matrix3 basis = Transpose(xform.GetBasis());
    return AffineTransform(basis, basis * -xform.GetTranslation());
  }

  inline OrthogonalTransform Invert(const OrthogonalTransform &xform)
  {
    return ~xform;
  }

  inline Matrix4 Transpose(const Matrix4 &mat)
  {
    return Matrix4(XMMatrixTranspose(mat));
  }

  inline Matrix4 Invert(const Matrix4 &mat)
  {
    return Matrix4(XMMatrixInverse(nullptr, mat));
  }

  inline Matrix4 OrthoInvert(const Matrix4 &xform)
  {
    Matrix3 basis = Transpose(xform.Get3x3());
    Vector3 translate = basis * -Vector3(xform.GetW());
    return Matrix4(basis, translate);
  }

}// namespace Neuron::Math