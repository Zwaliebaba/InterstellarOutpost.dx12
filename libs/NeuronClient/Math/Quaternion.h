
#pragma once

#include "Vector.h"

namespace Neuron::Math
{
  class Quaternion
  {
  public:
    inline Quaternion() { m_vec = XMQuaternionIdentity(); }

    inline Quaternion(const Vector3 &axis, const Scalar &angle) { m_vec = XMQuaternionRotationAxis(axis, angle); }

    inline Quaternion(float pitch, float yaw, float roll) { m_vec = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll); }

    inline explicit Quaternion(const XMMATRIX &matrix) { m_vec = XMQuaternionRotationMatrix(matrix); }

    inline explicit Quaternion(FXMVECTOR vec) { m_vec = vec; }

    inline explicit Quaternion(EIdentityTag) { m_vec = XMQuaternionIdentity(); }

    inline operator XMVECTOR() const { return m_vec; }

    inline Quaternion operator~(void) const { return Quaternion(XMQuaternionConjugate(m_vec)); }

    inline Quaternion operator-(void) const { return Quaternion(XMVectorNegate(m_vec)); }

    inline Quaternion operator*(Quaternion rhs) const { return Quaternion(XMQuaternionMultiply(rhs, m_vec)); }

    inline Vector3 operator*(Vector3 rhs) const { return Vector3(XMVector3Rotate(rhs, m_vec)); }

    inline Quaternion &operator=(Quaternion rhs)
    {
      m_vec = rhs;
      return *this;
    }

    inline Quaternion &operator*=(Quaternion rhs)
    {
      *this = *this * rhs;
      return *this;
    }

  protected:
    XMVECTOR m_vec;
  };

  inline Quaternion Normalize(Quaternion q)
  {
    return Quaternion(XMQuaternionNormalize(q));
  }

  inline Quaternion Slerp(Quaternion a, Quaternion b, float t)
  {
    return Normalize(Quaternion(XMQuaternionSlerp(a, b, t)));
  }

  inline Quaternion Lerp(Quaternion a, Quaternion b, float t)
  {
    return Normalize(Quaternion(XMVectorLerp(a, b, t)));
  }
}// namespace Neuron::Math
