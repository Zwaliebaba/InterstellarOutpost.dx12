#pragma once

class Camera
{
public:
  Camera();
  ~Camera() = default;

  void Get3DViewProjMatrices(XMFLOAT4X4 *view, XMFLOAT4X4 *proj, float fovInDegrees, float screenWidth, float screenHeight) const;
  void Reset();
  void Set(XMVECTOR eye, XMVECTOR at, XMVECTOR up);
  void RotateYaw(float deg);
  void RotatePitch(float deg);
  void GetOrthoProjMatrices(XMFLOAT4X4 *view, XMFLOAT4X4 *proj, float width, float height);

  // Z increases into of the screen when using LH coord system (which we are and DX uses)
  XMVECTOR m_eye = Vector3::ZERO;// Where the camera is in world space.
  XMVECTOR m_at = Vector3::ZERO; // What the camera is looking at (world origin)
  XMVECTOR m_up = Vector3::UP;   // Which way is up
};
