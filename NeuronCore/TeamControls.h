#ifndef TEAM_CONTROLS_H
#define TEAM_CONTROLS_H

#include "vector3.h"

// ****************************************************************************
//  Class TeamControls
//
//  Shared network contract: captures control information sent over the wire.
//  Extracted from team.h so that transport code (clienttoserver.h, server.h)
//  can depend on this without pulling in the full Team/Entity/WorldObject chain.
// ****************************************************************************

class TeamControls
{
public:
  TeamControls();

  unsigned short GetFlags() const;
  void SetFlags(unsigned short _flags);
  void ClearFlags();
  void Advance(); // implemented in InterstellarOutpost/team.cpp (client-side input)
  void Clear();

  void Pack(char* _data, int& _length) const;
  void Unpack(const char* _data, int _length);

  Vector3 m_mousePos;

  // Be sure to update GetFlags, SetFlags, ClearFlags if you change these flags
  // Also, NetworkUpdate::GetByteStream and NetworkUpdate::ReadByteStream
  // if you use more than 8 bits

  unsigned int m_unitMove : 1;
  unsigned int m_primaryFireTarget : 1;
  unsigned int m_secondaryFireTarget : 1;
  unsigned int m_targetDirected : 1;
  unsigned int m_secondaryFireDirected : 1;
  unsigned int m_cameraEntityTracking : 1;
  unsigned int m_directUnitMove : 1;
  unsigned int m_unitSecondaryMode : 1;
  unsigned int m_endSetTarget : 1;
  unsigned int m_consoleController : 1;
  unsigned int m_leftButtonPressed : 1;

  short m_directUnitMoveDx;
  short m_directUnitMoveDy;
  short m_directUnitTargetDx;
  short m_directUnitTargetDy;
};

#endif // TEAM_CONTROLS_H
