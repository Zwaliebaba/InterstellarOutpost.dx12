#include "pch.h"
#include "net_socket.h"
#include "net_socket_session.h"
#include "servertoclient.h"
#include "ftp_manager.h"
#include "directory.h"

ServerToClient::ServerToClient(char* _ip, int _port, NetSocketListener* _listener)
  : m_port(_port),
    m_socket(nullptr),
    m_clientId(-1),
    m_ftpManager(nullptr),
    m_spectator(false),
    m_lastKnownSequenceId(-1),
    m_lastSentSequenceId(-1),
    m_caughtUp(false),
    m_lastMessageReceived(GetHighResTime()),
    m_syncErrorSeqId(-1),
    m_disconnected(-1),
    m_lastAlive(nullptr),
    m_lastSelectUnit(nullptr),
    m_lastTeamColour(nullptr)
{
  strcpy(m_ip, _ip);
  m_socket = new NetSocketSession(*_listener, _ip, _port);
}

ServerToClient::~ServerToClient()
{
  delete m_ftpManager;
  delete m_lastAlive;
  delete m_lastSelectUnit;
  delete m_lastTeamColour;
  delete m_socket;
  m_socket = nullptr;

  m_ftpManager = nullptr;
  m_lastAlive = nullptr;
  m_lastSelectUnit = nullptr;
  m_lastTeamColour = nullptr;
}

char* ServerToClient::GetIP() { return m_ip; }

NetSocketSession* ServerToClient::GetSocket() { return m_socket; }

FTPManager* ServerToClient::GetFTPManager()
{
  if (!m_ftpManager)
  {
    char name[64];
    sprintf(name, "SERVER:%d", m_clientId);
    m_ftpManager = new FTPManager(name);
  }

  return m_ftpManager;
}
