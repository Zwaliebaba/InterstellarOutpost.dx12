#include "pch.h"
#include "math_utils.h"
#include "resource.h"
#include "shape.h"
#include "entity.h"
#include "entity_leg.h"
#include "app.h"
#include "camera.h"
#include "location.h"
#include "main.h"

EntityLeg::EntityLeg(int _legNum, Entity* _parent, const char* _shapeNameUpper, const char* _shapeNameLower, const char* _rootMarkerName)
  : m_legNum(_legNum),
    m_parent(_parent)
{
  m_shapeUpper = g_app->m_resource->GetShape(_shapeNameUpper);
  m_shapeLower = g_app->m_resource->GetShape(_shapeNameLower);
  ASSERT_TEXT(m_shapeUpper, "EntityLeg: Couldn't load leg shape %s", _shapeNameUpper);
  ASSERT_TEXT(m_shapeLower, "EntityLeg: Couldn't load leg shape %s", _shapeNameLower);

  constexpr char endMarkerName[] = "MarkerEnd";
  ShapeMarker* endMarker = m_shapeUpper->m_rootFragment->LookupMarker(endMarkerName);
  ASSERT_TEXT(endMarker, "EntityLeg: Can't get Marker(%s) from shape(%s), probably a corrupted file\n", endMarkerName,
              m_shapeUpper->m_name);

  const Matrix34& endMatrix = endMarker->GetWorldMatrix(Matrix34(0));
  m_thighLen = endMatrix.pos.Mag();

  endMarker = m_shapeLower->m_rootFragment->LookupMarker(endMarkerName);
  ASSERT_TEXT(endMarker, "EntityLeg: Can't get Marker(%s) from shape(%s), probably a corrupted file\n", endMarkerName,
              m_shapeLower->m_name);

  const Matrix34& endMatrixLower = endMarker->GetWorldMatrix(Matrix34(0));
  m_shinLen = endMatrixLower.pos.Mag();

  m_rootMarker = m_parent->m_shape->m_rootFragment->LookupMarker(_rootMarkerName);
  ASSERT_TEXT(m_rootMarker, "EntityLeg: Can't get Marker(%s) from shape(%s), probably a corrupted file\n", _rootMarkerName,
              m_parent->m_shape->m_name);

  m_foot.m_state = EntityFoot::OnGround;
}

Vector3 EntityLeg::GetLegRootPos()
{
  Matrix34 rootMat(m_parent->m_front, g_upVector, m_parent->m_pos);
  const Matrix34& resultMat = m_rootMarker->GetWorldMatrix(rootMat);

  return resultMat.pos;
}

Vector3 EntityLeg::CalcFootHomePos(double _targetHoverHeight)
{
  Vector3 rootWorldPos = GetLegRootPos();

  Vector3 fromCentreToRoot = rootWorldPos - m_parent->m_pos;
  fromCentreToRoot.HorizontalAndNormalise();

  Vector3 groundNormal = g_app->m_location->m_landscape.m_normalMap->GetValue(m_parent->m_pos.x, m_parent->m_pos.z);
  fromCentreToRoot *= groundNormal.y;

  Vector3 returnVal = rootWorldPos;
  returnVal += fromCentreToRoot * (m_idealLegSlope * _targetHoverHeight);
  returnVal.y -= _targetHoverHeight;

  return returnVal;
}

double EntityLeg::CalcFootsDesireToMove(double _targetHoverHeight)
{
  Vector3 homePos = CalcFootHomePos(_targetHoverHeight);
  Vector3 delta = m_foot.m_pos - homePos;
  Vector3 deltaHoriNorm = delta;
  deltaHoriNorm.HorizontalAndNormalise();

  double scoreDueToDirection = 1.0;
  double scoreDueToDist = delta.Mag();
  double score = scoreDueToDirection * scoreDueToDist;

  return score;
}

Vector3 EntityLeg::CalcDesiredFootPos(double _targetHoverHeight)
{
  Vector3 rv = CalcFootHomePos(_targetHoverHeight);

  double expectedRotation = 1.2 * m_parent->m_angVel.y;
  Vector3 averageExpectedVel = m_parent->m_vel;
  averageExpectedVel.RotateAroundY(expectedRotation * 0.5);
  rv += averageExpectedVel * m_lookAheadCoef;

  return rv;
}

Vector3 EntityLeg::CalcKneePos(const Vector3& _footPos, const Vector3& _rootPos, const Vector3& _centrePos)
{
  Vector3 rootToFoot(_footPos - _rootPos);
  double rootToFootLen = rootToFoot.Mag();

  Vector3 rootToFootHoriNorm(rootToFoot);
  rootToFootHoriNorm.HorizontalAndNormalise();
  Vector3 centreToRoot(_rootPos - _centrePos);
  centreToRoot.HorizontalAndNormalise();
  Vector3 axis((centreToRoot ^ g_upVector).Normalise());
  double cosTheta = (rootToFootLen * 0.4) / m_thighLen;
  // FIXME
  // cosTheta should never be greater than one, yet sometimes it is
  if (cosTheta > 1.0)
    cosTheta = 1.0;
  double theta = -acos(cosTheta);

  Vector3 footToKnee(-rootToFoot);
  footToKnee.SetLength(m_shinLen);
  footToKnee.RotateAround(axis * theta);

  return _footPos + footToKnee;
}

void EntityLeg::LiftFoot(double _targetHoverHeight)
{
  m_foot.m_targetPos = CalcDesiredFootPos(_targetHoverHeight);
  m_foot.m_targetPos.y = g_app->m_location->m_landscape.m_heightMap->GetValue(m_foot.m_targetPos.x, m_foot.m_targetPos.z);
  m_foot.m_state = EntityFoot::Swinging;
  m_foot.m_leftGroundTimeStamp = g_gameTime;
  m_foot.m_lastGroundPos = m_foot.m_pos;
}

void EntityLeg::PlantFoot()
{
  m_foot.m_pos = m_foot.m_targetPos;
  m_foot.m_state = EntityFoot::OnGround;
}

Vector3 EntityLeg::GetIdealSwingingFootPos(double _fractionComplete)
{
  double fractionIncomplete = 1.0 - _fractionComplete;
  Vector3 pos = m_foot.m_lastGroundPos * fractionIncomplete + m_foot.m_targetPos * _fractionComplete;

  if (_fractionComplete < 0.33)
    pos.y += _fractionComplete * 3.0 * m_legLift;
  else if (fractionIncomplete < 0.33)
    pos.y += fractionIncomplete * 3.0 * m_legLift;
  else
    pos.y += m_legLift;

  return pos;
}

// Returns true if the foot was planted this frame
bool EntityLeg::Advance()
{
  bool result = false;
  if (m_foot.m_state == EntityFoot::Swinging)
  {
    double fractionComplete = RampUpAndDown(m_foot.m_leftGroundTimeStamp, m_legSwingDuration, g_gameTime);
    if (fractionComplete > 1.0)
    {
      PlantFoot();
      result = true;
    }
    else
      m_foot.m_pos = GetIdealSwingingFootPos(fractionComplete);
  }
  return result;
}

void EntityLeg::AdvanceSpiderPounce(double _fractionComplete)
{
  Vector3 offset = m_foot.m_bodyToFoot;
  if (_fractionComplete < 0.5)
    offset *= 1.0 - _fractionComplete;
  else
    offset *= _fractionComplete;
  m_foot.m_pos = m_parent->m_pos - offset + m_parent->m_vel * _fractionComplete * 0.05;
  m_foot.m_lastGroundPos = m_foot.m_pos;
  m_foot.m_targetPos = m_foot.m_pos;
}

void EntityLeg::Render(double _predictionTime, const Vector3& _predictedMovement)
{
  Vector3 predictedPos = m_parent->m_pos + _predictedMovement;
  Vector3 rootPos(_predictedMovement + GetLegRootPos());
  Vector3 footPos;

  switch (m_foot.m_state)
  {
  case EntityFoot::OnGround:
    footPos = m_foot.m_pos;
    break;

  case EntityFoot::Swinging:
  {
    double fractionComplete = RampUpAndDown(m_foot.m_leftGroundTimeStamp, m_legSwingDuration, g_gameTime);
    footPos = GetIdealSwingingFootPos(fractionComplete);
    break;
  }

  case EntityFoot::Pouncing:
    footPos = m_foot.m_pos + _predictedMovement;
    break;
  }

  Vector3 kneePos = CalcKneePos(footPos, rootPos, predictedPos);

  {
    Vector3 up((kneePos - footPos).Normalise());
    Vector3 front(up ^ g_upVector);
    Matrix34 mat(front, up, footPos);
    m_shapeLower->Render(_predictionTime, mat);
  }

  {
    Vector3 up((rootPos - kneePos).Normalise());
    Vector3 front(up ^ g_upVector);
    Matrix34 mat(front, up, kneePos);
    m_shapeUpper->Render(_predictionTime, mat);
  }
}
