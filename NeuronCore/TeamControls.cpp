#include "pch.h"
#include "TeamControls.h"
#include "network_stream.h"

#include <strstream>

// ****************************************************************************
//  Class TeamControls — pure data / serialisation methods
//  Advance() lives in InterstellarOutpost/team.cpp (client-side input gathering)
// ****************************************************************************

TeamControls::TeamControls() { Clear(); }

unsigned short TeamControls::GetFlags() const
{
  return (m_unitMove ? 0x0001 : 0) | (m_directUnitMove ? 0x0002 : 0) | (m_primaryFireTarget ? 0x0004 : 0) | (
    m_secondaryFireTarget ? 0x0008 : 0) | (m_targetDirected ? 0x0010 : 0) | (m_secondaryFireDirected ? 0x0020 : 0) | (
    m_cameraEntityTracking ? 0x0040 : 0) | (m_unitSecondaryMode ? 0x0080 : 0) | (m_endSetTarget ? 0x0100 : 0) | (
    m_consoleController ? 0x0200 : 0) | (m_leftButtonPressed ? 0x0400 : 0);
}

void TeamControls::SetFlags(unsigned short _flags)
{
  m_unitMove = _flags & 0x0001 ? 1 : 0;
  m_directUnitMove = _flags & 0x0002 ? 1 : 0;
  m_primaryFireTarget = _flags & 0x0004 ? 1 : 0;
  m_secondaryFireTarget = _flags & 0x0008 ? 1 : 0;
  m_targetDirected = _flags & 0x0010 ? 1 : 0;
  m_secondaryFireDirected = _flags & 0x0020 ? 1 : 0;
  m_cameraEntityTracking = _flags & 0x0040 ? 1 : 0;
  m_unitSecondaryMode = _flags & 0x0080 ? 1 : 0;
  m_endSetTarget = _flags & 0x0100 ? 1 : 0;
  m_consoleController = _flags & 0x0200 ? 1 : 0;
  m_leftButtonPressed = _flags & 0x0400 ? 1 : 0;
}

void TeamControls::Clear() { memset(this, 0, sizeof(*this)); }

void TeamControls::ClearFlags()
{
  m_unitMove = 0;
  m_directUnitMove = 0;
  m_primaryFireTarget = 0;
  m_secondaryFireTarget = 0;
  m_targetDirected = 0;
  m_secondaryFireDirected = 0;
  m_cameraEntityTracking = 0;
  m_unitSecondaryMode = 0;
  m_endSetTarget = 0;
  m_consoleController = 0;
  m_leftButtonPressed = 0;
}

void TeamControls::Pack(char* _data, int& _length) const
{
  std::ostrstream s(_data, 256);

  WriteNetworkValue(s, GetFlags());
  if (m_directUnitMove)
  {
    WriteNetworkValue(s, m_directUnitMoveDx);
    WriteNetworkValue(s, m_directUnitMoveDy);
  }

  if (m_targetDirected || m_secondaryFireDirected)
  {
    WriteNetworkValue(s, m_directUnitTargetDx);
    WriteNetworkValue(s, m_directUnitTargetDy);
  }

  WriteNetworkValue(s, m_mousePos);
  _length = s.tellp();
}

void TeamControls::Unpack(const char* _data, int _length)
{
  std::istrstream s(_data, _length);

  unsigned short flags;
  ReadNetworkValue(s, flags);
  SetFlags(flags);

  if (m_directUnitMove)
  {
    ReadNetworkValue(s, m_directUnitMoveDx);
    ReadNetworkValue(s, m_directUnitMoveDy);
  }

  if (m_targetDirected || m_secondaryFireDirected)
  {
    ReadNetworkValue(s, m_directUnitTargetDx);
    ReadNetworkValue(s, m_directUnitTargetDy);
  }

  ReadNetworkValue(s, m_mousePos);
}
