#include "pch.h"

#include "Camera.h"

Camera::Camera()
{
  Reset();
}

void Camera::Get3DViewProjMatrices(XMFLOAT4X4 *view, XMFLOAT4X4 *proj, float fovInDegrees, float screenWidth, float screenHeight) const
{

  float aspectRatio = (float) screenWidth / (float) screenHeight;
  float fovAngleY = fovInDegrees * XM_PI / 180.0F;

  if (aspectRatio < 1.0F) { fovAngleY /= aspectRatio; }

  XMStoreFloat4x4(view, XMMatrixTranspose(XMMatrixLookAtRH(m_eye, m_at, m_up)));
  XMStoreFloat4x4(proj, XMMatrixTranspose(XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 125.0f)));
}

void Camera::GetOrthoProjMatrices(XMFLOAT4X4 *view, XMFLOAT4X4 *proj, float width, float height)
{
  XMStoreFloat4x4(view, XMMatrixTranspose(XMMatrixLookAtRH(m_eye, m_at, m_up)));
  XMStoreFloat4x4(proj, XMMatrixTranspose(XMMatrixOrthographicRH(width, height, 0.01F, 125.0F)));
}
void Camera::RotateYaw(float deg)
{
  XMMATRIX rotation = XMMatrixRotationAxis(m_up, deg);

  m_eye = XMVector3TransformCoord(m_eye, rotation);
}

void Camera::RotatePitch(float deg)
{
  XMVECTOR right = XMVector3Normalize(XMVector3Cross(m_eye, m_up));
  XMMATRIX rotation = XMMatrixRotationAxis(right, deg);

  m_eye = XMVector3TransformCoord(m_eye, rotation);
}

void Camera::Reset()
{
  m_eye = XMVectorSet(0.0F, 15.0F, -30.0F, 0.0F);
  m_at = XMVectorSet(0.0F, 8.0F, 0.0F, 0.0F);
}

void Camera::Set(XMVECTOR eye, XMVECTOR at, XMVECTOR up)
{
  m_eye = eye;
  m_at = at;
  m_up = up;
}
