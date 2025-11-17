
#pragma once

#include "Common.h"

namespace Neuron::Math
{
  class Scalar
  {
  public:
    inline Scalar() {}

    inline Scalar(const Scalar &s) { m_vec = s; }

    inline Scalar(float f) { m_vec = XMVectorReplicate(f); }

    inline explicit Scalar(FXMVECTOR vec) { m_vec = vec; }

    inline explicit Scalar(EZeroTag) { m_vec = SplatZero(); }

    inline explicit Scalar(EIdentityTag) { m_vec = SplatOne(); }

    inline operator XMVECTOR() const { return m_vec; }

    inline operator float() const { return XMVectorGetX(m_vec); }

  private:
    XMVECTOR m_vec;
  };

  inline Scalar operator-(Scalar s)
  {
    return Scalar(XMVectorNegate(s));
  }

  inline Scalar operator+(Scalar s1, Scalar s2)
  {
    return Scalar(XMVectorAdd(s1, s2));
  }

  inline Scalar operator-(Scalar s1, Scalar s2)
  {
    return Scalar(XMVectorSubtract(s1, s2));
  }

  inline Scalar operator*(Scalar s1, Scalar s2)
  {
    return Scalar(XMVectorMultiply(s1, s2));
  }

  inline Scalar operator/(Scalar s1, Scalar s2)
  {
    return Scalar(XMVectorDivide(s1, s2));
  }

  inline Scalar operator+(Scalar s1, float s2)
  {
    return s1 + Scalar(s2);
  }

  inline Scalar operator-(Scalar s1, float s2)
  {
    return s1 - Scalar(s2);
  }

  inline Scalar operator*(Scalar s1, float s2)
  {
    return s1 * Scalar(s2);
  }

  inline Scalar operator/(Scalar s1, float s2)
  {
    return s1 / Scalar(s2);
  }

  inline Scalar operator+(float s1, Scalar s2)
  {
    return Scalar(s1) + s2;
  }

  inline Scalar operator-(float s1, Scalar s2)
  {
    return Scalar(s1) - s2;
  }

  inline Scalar operator*(float s1, Scalar s2)
  {
    return Scalar(s1) * s2;
  }

  inline Scalar operator/(float s1, Scalar s2)
  {
    return Scalar(s1) / s2;
  }

}// namespace Neuron::Math
