#include "pch.h"
#include "darray.h"
#include "directory.h"
#include "net_lib.h"

#include "hi_res_time.h"

#include "matchmaker.h"
#include "metaserver.h"
#include "metaserver_defines.h"

// Static data for MatchMaker

static char* s_matchMakerIp = nullptr;
static int s_matchMakerPort = -1;

static NetMutex s_uniqueRequestMutex;
static int s_uniqueRequestid = 0;

struct MatchMakerListener
{
  MatchMakerListener()
    : m_listener(nullptr),
      m_ip(nullptr),
      m_uniqueId(-1),
      m_port(-1),
      m_identified(false),
      m_shutDown(false) {}

  NetSocketListener* m_listener;
  char* m_ip;
  int m_uniqueId;
  int m_port;
  bool m_identified;
  bool m_shutDown;
};

static DArray<MatchMakerListener*> s_listeners;
static NetMutex s_listenersMutex;


void MatchMaker_LocateService(const char* _matchMakerIp, int _port)
{
  if (s_matchMakerIp)
  {
    delete s_matchMakerIp;
    s_matchMakerIp = nullptr;
  }

  if (_matchMakerIp)
  {
    s_matchMakerIp = _strdup(_matchMakerIp);
    s_matchMakerPort = _port;
  }
}

static int GetListenerIndex(NetSocketListener* _listener, const NetLockMutex& _lockSListeners)
{
  for (int i = 0; i < s_listeners.Size(); ++i)
  {
    if (s_listeners.ValidIndex(i))
    {
      MatchMakerListener* listener = s_listeners[i];
      if (listener->m_listener == _listener)
        return i;
    }
  }

  return -1;
}

static void RemoveListener(MatchMakerListener* _listener, NetLockMutex& _lockSListeners)
{
  for (int i = 0; i < s_listeners.Size(); ++i)
  {
    if (s_listeners.ValidIndex(i) && s_listeners[i] == _listener)
    {
      s_listeners.RemoveData(i);
      return;
    }
  }
}

static MatchMakerListener* GetListener(NetSocketListener* _listener, const NetLockMutex& _lockSListeners)
{
  int listenerIndex = GetListenerIndex(_listener, _lockSListeners);
  if (listenerIndex == -1)
    return nullptr;

  return s_listeners[listenerIndex];
}

static NetCallBackRetType RequestIdentityThread(void* ignored)
{
  return 0;
}

static bool IsRequestingIdentity(NetSocketListener* _listener, NetLockMutex& _lock)
{
  MatchMakerListener* listener = GetListener(_listener, _lock);
  if (!listener)
    return false;
  if (listener->m_shutDown)
    return false;

  return true;
}

bool MatchMaker_IsRequestingIdentity(NetSocketListener* _listener)
{
  NetLockMutex lock(s_listenersMutex);
  return IsRequestingIdentity(_listener, lock);
}

void MatchMaker_StartRequestingIdentity(NetSocketListener* _listener)
{
  NetLockMutex lock(s_listenersMutex);

  ASSERT(s_matchMakerIp);

  if (_listener && !IsRequestingIdentity(_listener, lock))
  {
    auto listener = new MatchMakerListener();
    listener->m_listener = _listener;
    int index = s_listeners.PutData(listener);

    DebugTrace("Started requesting public IP:port for socket %d ({:x})\n", _listener->GetBoundSocketHandle(), (size_t)listener);

    NetStartThread(RequestIdentityThread, _listener);
  }
}

void MatchMaker_StopRequestingIdentity(NetSocketListener* _listener)
{
  NetLockMutex lock(s_listenersMutex);

  MatchMakerListener* listener = GetListener(_listener, lock);

  if (listener)
  {
    DebugTrace("Stopped requesting public IP:port for socket %d ({:x})\n", _listener->GetBoundSocketHandle(), (size_t)listener);

    listener->m_shutDown = true;
    RemoveListener(listener, lock);
  }
}

bool MatchMaker_IsIdentityKnown(NetSocketListener* _listener)
{
  NetLockMutex lock(s_listenersMutex);

  MatchMakerListener* listener = GetListener(_listener, lock);

  if (!listener)
    return false;

  return (listener->m_identified);
}

bool MatchMaker_GetIdentity(NetSocketListener* _listener, char* _ip, int* _port)
{
  NetLockMutex lock(s_listenersMutex);

  MatchMakerListener* listener = GetListener(_listener, lock);

  if (!listener)
    return false;

  if (!listener->m_identified)
    return false;

  strcpy(_ip, listener->m_ip);
  *_port = listener->m_port;

  return true;
}

void MatchMaker_RequestConnection(NetSocketListener* _listener, const char* _targetIp, int _targetPort, Directory* _myDetails)
{
}

bool MatchMaker_ReceiveMessage(NetSocketListener* _listener, Directory* _message)
{
  ASSERT(_message);
  ASSERT(strcmp(_message->m_name, NET_MATCHMAKER_MESSAGE) == 0);
  ASSERT(_message->HasData( NET_METASERVER_COMMAND, DIRECTORY_TYPE_STRING ));

  char* cmd = _message->GetDataString(NET_METASERVER_COMMAND);

  if (strcmp(cmd, NET_MATCHMAKER_IDENTIFY) == 0)
  {
    //
    // This message contains the external IP and port of one of our connections

    if (!_message->HasData(NET_MATCHMAKER_UNIQUEID, DIRECTORY_TYPE_INT) || !_message->HasData(NET_MATCHMAKER_YOURIP, DIRECTORY_TYPE_STRING)
      || !_message->HasData(NET_MATCHMAKER_YOURPORT, DIRECTORY_TYPE_INT))
      DebugTrace("MatchMaker : Received badly formed identity message, discarded\n");
    else
    {
      int uniqueId = _message->GetDataInt(NET_MATCHMAKER_UNIQUEID);

      s_listenersMutex.Lock();
      for (int i = 0; i < s_listeners.Size(); ++i)
      {
        if (s_listeners.ValidIndex(i))
        {
          MatchMakerListener* listener = s_listeners[i];
          if (listener->m_uniqueId == uniqueId)
          {
            if (!listener->m_identified)
            {
              listener->m_ip = _strdup(_message->GetDataString(NET_MATCHMAKER_YOURIP));
              listener->m_port = _message->GetDataInt(NET_MATCHMAKER_YOURPORT);
              listener->m_identified = true;
              DebugTrace("Socket %d identified as public IP %s:%d\n", listener->m_listener->GetBoundSocketHandle(), listener->m_ip,
                         listener->m_port);
            }
            break;
          }
        }
      }
      s_listenersMutex.Unlock();
    }
  }
  else if (strcmp(cmd, NET_MATCHMAKER_REQUEST_CONNECT) == 0)
  {
    //
    // This is a request from a client for the server to set up a UDP hole punch

    if (!_message->HasData(NET_METASERVER_IP, DIRECTORY_TYPE_STRING) || !_message->HasData(NET_METASERVER_PORT, DIRECTORY_TYPE_INT))
      DebugTrace("MatchMaker : Received badly formed connection request, discarded\n");
    else
    {
      char* ip = _message->GetDataString(NET_METASERVER_IP);
      int port = _message->GetDataInt(NET_METASERVER_PORT);

      DebugTrace("MatchMaker : SERVER Received request to allow client to join from %s:%d\n", ip, port);

      Directory dir;
      dir.SetName(NET_MATCHMAKER_MESSAGE);
      dir.CreateData(NET_METASERVER_COMMAND, NET_MATCHMAKER_RECEIVED_CONNECT);

      NetSocketSession session(*_listener, ip, port);
      MetaServer_SendDirectory(&dir, &session);
    }
  }
  else if (strcmp(cmd, NET_MATCHMAKER_RECEIVED_CONNECT) == 0)
    DebugTrace("MatchMaker : CLIENT received confirmation from Server that Hole Punch is set up\n");
  else
    Fatal("unrecognised matchmaker message");

  return true;
}
