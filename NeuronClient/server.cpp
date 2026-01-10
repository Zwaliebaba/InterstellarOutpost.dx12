#include "pch.h"
#include <fstream>
#include "net_lib.h"
#include "net_mutex.h"
#include "net_socket.h"
#include "net_socket_listener.h"
#include "net_thread.h"
#include "net_udp_packet.h"
#include "hi_res_time.h"
#include "profiler.h"
#include "preferences.h"
#include "hash.h"
#include "app.h"
#include "globals.h"
#include "team.h"
#include "multiwinia.h"
#include "generic.h"
#include "server.h"
#include "servertoclient.h"
#include "servertoclientletter.h"
#include "clienttoserver.h"
#include "network_defines.h"
#include "ftp_manager.h"
#include "metaserver_defines.h"
#include "game_menu.h"
#include "soundsystem.h"
#include "MapData.h"
#include "authentication.h"

// ****************************************************************************
// Class ServerTeam
// ****************************************************************************

ServerTeam::ServerTeam(int _clientId, int _teamType)
  : m_clientId(_clientId),
    m_teamType(_teamType) {}

// ****************************************************************************
// Class Server
// ****************************************************************************

// ***ListenCallback
static NetCallBackRetType ServerListenCallback(NetUdpPacket* udpdata)
{
  if (!udpdata)
    return 0;

  Server* server = g_app->m_server;

  if (!server)
  {
    delete udpdata;
    return 0;
  }

  NetIpAddress* fromAddr = &udpdata->m_clientAddress;
  char newip[16];
  IpToString(fromAddr->sin_addr, newip);
  int newPort = udpdata->GetPort();

  auto letter = new Directory();
  bool success = letter->Read(udpdata->m_data, udpdata->m_length);
  if (success)
    server->ReceiveLetter(letter, newip, newPort, udpdata->m_length);
  else
  {
    DebugTrace("Server received bogus letter, discarded (10)\n");
    delete letter;
  }

  delete udpdata;
  return 0;
}

Server::Server()
  : m_startTime(-1.0),
    m_nextServerAdvanceTime(0),
    m_random(0),
    m_syncRandSeed(0),
    m_gameStartMsgReceived(false),
    m_sequenceId(0),
    m_nextClientId(0),
    m_inboxMutex(nullptr),
    m_outboxMutex(nullptr),
    m_noAdvertise(false),
    m_listener(nullptr) {}

Server::~Server()
{
  DebugTrace("Shutting down server\n");

  m_listener->StopListening();
  delete m_listener;

  m_history.EmptyAndDelete();
  m_clients.EmptyAndDelete();
  m_disconnectedClients.EmptyAndDelete();
  m_teams.EmptyAndDelete();

  m_inboxMutex->Lock();
  m_inbox.EmptyAndDelete();
  m_inboxMutex->Unlock();

  m_outboxMutex->Lock();
  m_outbox.EmptyAndDelete();
  m_outboxMutex->Unlock();
}

static NetCallBackRetType ListenThread(void* ptr)
{
  auto server = static_cast<Server*>(ptr);
  server->StartListening();
  return 0;
}

void Server::StartListening() { m_listener->StartListening(ServerListenCallback); }

static unsigned MakeRandom()
{
  unsigned threadId = GetCurrentThreadId();
  double theTime = GetHighResTime();
  return threadId ^ *(unsigned*)&theTime;
}

static int GenSyncRandSeed(double _startTime, unsigned _random)
{
  unsigned int hash[5];
  hash_context c;

  hash_initial(&c);
  Hash(c, _random);
  Hash(c, _startTime);
  hash_final(&c, hash);

  return static_cast<int>(hash[0]);
}

void Server::Initialise()
{
  m_iClientCount = 0;

  m_inboxMutex = new NetMutex();
  m_outboxMutex = new NetMutex();

  int ourPort = g_prefsManager->GetInt(PREFS_NETWORKSERVERPORT, 4000);

  m_listener = new NetSocketListener(ourPort, "Server");
  NetRetCode result = m_listener->Bind();

  if (result != NetOk)
  {
    // TODO PORT FORWARDING: 		
    // We need to indicate to the user that his chosen port is unavailable instead of 
    // just dynamically allocating one
  }

  NetStartThread(ListenThread, this);

  m_startTimeActual = time(nullptr);
  m_startTime = GetHighResTime();
  m_random = MakeRandom();
  m_syncRandSeed = GenSyncRandSeed(m_startTime, m_random);
  m_gameStartMsgReceived = false;

  m_nextServerAdvanceTime = m_startTime + 0.1;
}

// *** GetClientId

ServerToClient* Server::GetClient(int _id)
{
  for (int i = 0; i < m_clients.Size(); ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* stc = m_clients[i];
      if (stc->m_clientId == _id)
        return stc;
    }
  }

  return nullptr;
}

bool Server::CheckPlayerCanJoin(Directory* _incoming) { return true; }

int Server::GetClientId(const char* _ip, int _port)
{
  for (int i = 0; i < m_clients.Size(); ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* sToC = m_clients[i];

      if (strcmp(sToC->m_ip, _ip) == 0 && sToC->m_port == _port)
        return m_clients[i]->m_clientId;
    }
  }

  return -1;
}

int Server::ConvertIPToInt(const char* _ip)
{
  ASSERT_TEXT(strlen(_ip) < 17, "IP address too long");
  char ipCopy[17];
  strcpy(ipCopy, _ip);
  int ipLen = strlen(ipCopy);

  for (int i = 0; i < ipLen; ++i)
  {
    if (ipCopy[i] == '.')
      ipCopy[i] = '\n';
  }

  int part1, part2, part3, part4;
  sscanf(ipCopy, "%d %d %d %d", &part1, &part2, &part3, &part4);

  int result = ((part4 & 0xff) << 24) + ((part3 & 0xff) << 16) + ((part2 & 0xff) << 8) + (part1 & 0xff);
  return result;
}

char* Server::ConvertIntToIP(const int _ip)
{
  static char result[16];
  sprintf(result, "%d.%d.%d.%d", (_ip & 0x000000ff), (_ip & 0x0000ff00) >> 8, (_ip & 0x00ff0000) >> 16, (_ip & 0xff000000) >> 24);

  return result;
}

int Server::RegisterNewClient(Directory* _client, int _clientId)
{
  char* ip = _client->GetDataString(NET_DARWINIA_FROMIP);
  int port = _client->GetDataInt(NET_DARWINIA_FROMPORT);

  ASSERT(GetClientId( ip, port ) == -1);

  auto sToC = new ServerToClient(ip, port, m_listener);

  strcpy(sToC->m_authKey, _client->GetDataString(NET_METASERVER_AUTHKEY));
  sToC->m_authKeyId = _client->GetDataInt(NET_METASERVER_AUTHKEYID);

  if (_clientId == -1)
  {
    sToC->m_clientId = m_nextClientId;
    m_nextClientId++;
  }
  else
    sToC->m_clientId = _clientId;

  if (_client->HasData(NET_DARWINIA_SERVER_PASSWORD))
    sToC->m_password = _client->GetDataUnicode(NET_DARWINIA_SERVER_PASSWORD);

  m_clients.PutData(sToC);

  //
  // Try to authenticate this person

  Authentication_RequestStatus(sToC->m_authKey, METASERVER_GAMETYPE_MULTIWINIA, sToC->m_authKeyId, ip);

  //
  // Tell all clients about it
  auto data = new Directory;
  data->SetName(NET_DARWINIA_MESSAGE);
  data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_CLIENTHELLO);
  data->CreateData(NET_DARWINIA_CLIENTID, sToC->m_clientId);

  // Tell about the start time and random number 
  data->CreateData(NET_METASERVER_STARTTIME, static_cast<unsigned long long>(m_startTimeActual));
  data->CreateData(NET_METASERVER_RANDOM, static_cast<int>(m_random));

  auto letter = new ServerToClientLetter();
  letter->m_data = data;
  SendLetter(letter);

  //
  // Return assigned clientID
  return sToC->m_clientId;
}

void Server::SendClientId(int _clientId)
{
  if (_clientId != -1)
  {
    ServerToClient* sToC = GetClient(_clientId);
    ASSERT(sToC);

    auto data = new Directory;
    data->SetName(NET_DARWINIA_MESSAGE);
    data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_CLIENTID);
    data->CreateData(NET_DARWINIA_CLIENTID, _clientId);
    data->CreateData(NET_DARWINIA_SEQID, -1);
    data->CreateData(NET_DARWINIA_VERSION, APP_VERSION);

    data->CreateData(NET_DARWINIA_RANDSEED, m_syncRandSeed);

    // At this point I am going to decide to make the player a spectator or not based on the fill status of the server

    if (g_app->GetMaxNumberofPlayers())
    {
      int iTotalPlayers = g_app->GetMaxNumberofPlayers();

      // We want to reject the player if the server is full
      if (iTotalPlayers != 0)
      {
        if (m_clients.NumUsed() > iTotalPlayers)
        {
          DebugTrace("Clients Used: %d, total players %d, making spectator\n", m_clients.NumUsed(), iTotalPlayers);
          data->CreateData(NET_DARWINIA_MAKE_SPECTATOR, 1);
        }
        else
          data->CreateData(NET_DARWINIA_MAKE_SPECTATOR, 0);
      }
    }

    auto letter = new ServerToClientLetter();
    letter->m_data = data;
    letter->m_receiverId = _clientId;

    m_outboxMutex->Lock();
    m_outbox.PutDataAtStart(letter);
    m_outboxMutex->Unlock();

    char fromStr[64];
    sprintf(fromStr, "%s:%d", sToC->m_ip, sToC->m_port);

    DebugTrace("SERVER: Client at %s requested ID.  Sent ID %d (syncrand %d).\n", fromStr, _clientId, m_syncRandSeed);
  }
}

void Server::RemoveClient(int _clientId, int _reason)
{
  for (int i = 0; i < m_clients.Size(); ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* sToC = m_clients[i];
      if (sToC->m_clientId == _clientId)
      {
        static const char* stringReasons[] = {"client left", "client dropped", "client needed pdlc", "server full", "invalid key",
                                              "duplicate key", "bad password", "demo full", "key authentication failed",
                                              "kicked by server"};

        const char* stringReason = stringReasons[_reason];

        char fromStr[64];
        sprintf(fromStr, "%s:%d", sToC->m_ip, sToC->m_port);

        DebugTrace("SERVER: Client at %s disconnected (%s)\n", fromStr, stringReason);
        //
        // Tell all clients about it

        auto letter = new ServerToClientLetter();

        letter->m_data = new Directory();
        letter->m_data->SetName(NET_DARWINIA_MESSAGE);
        letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_CLIENTGOODBYE);
        letter->m_data->CreateData(NET_DARWINIA_CLIENTID, sToC->m_clientId);
        letter->m_data->CreateData(NET_DARWINIA_DISCONNECT, _reason);

        SendLetter(letter);

        //
        // Tell the client specifically
        // (Unless he's telling us he left)

        if (_reason != Disconnect_ClientLeave)
        {
          DebugTrace("The client did not leave, so sending them a message\n");
          letter = new ServerToClientLetter();
          letter->m_data = new Directory();
          letter->m_data->SetName(NET_DARWINIA_MESSAGE);
          letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_DISCONNECT);
          letter->m_data->CreateData(NET_DARWINIA_DISCONNECT, _reason);
          letter->m_receiverId = sToC->m_clientId;
          letter->m_data->CreateData(NET_DARWINIA_SEQID, -1);

          m_outbox.PutDataAtEnd(letter);
        }

        AdvanceSender();

        m_clients.RemoveData(i);
        m_disconnectedClients.PutData(sToC);
        sToC->m_disconnected = _reason;

        //
        // Remove his teams if we're in the lobby

        if (g_app->m_multiwinia->GameInLobby())
        {
          for (int j = 0; j < m_teams.Size(); j++)
          {
            if (m_teams.ValidIndex(j))
            {
              ServerTeam* st = m_teams[j];
              if (st->m_clientId == _clientId)
              {
                DebugTrace("Removing server team %d for client %d\n", j, _clientId);
                m_teams.RemoveData(j);
                delete st;
                j--;
              }
            }
          }
        }

        break;
      }
    }
  }
}

void Server::RegisterNewTeam(int _clientId, int _teamType, int _desiredTeamId)
{

  ServerToClient* sToC = GetClient(_clientId);
  ASSERT(sToC);

  // Ignore team requests from spectators
  if (sToC->m_spectator)
    return;

  //
  // If we already have a local team do something different

  if (_teamType == TeamTypeLocalPlayer)
  {
    for (int i = 0; i < m_teams.Size(); ++i)
    {
      if (m_teams.ValidIndex(i))
      {
        ServerTeam* team = m_teams[i];
        if (team->m_clientId == _clientId && team->m_teamType == TeamTypeLocalPlayer)
        {
          DebugTrace("SERVER: Failed to create new team - this client already has a Team\n");
          return;
        }
      }
    }

    // Single player
    if (g_app->m_clientToServer->GetServerPort() == -1)
    {
      int nbPlayerTeams = 0;
      for (int i = 0; i < m_teams.Size(); ++i)
      {
        if (m_teams.ValidIndex(i))
        {
          ServerTeam* team = m_teams[i];
          if (team->m_teamType == TeamTypeLocalPlayer || team->m_teamType == TeamTypeRemotePlayer)
            nbPlayerTeams++;
        }
      }

      if (nbPlayerTeams > 0)
      {
        // Remove the client
        RemoveClient(_clientId, Disconnect_ServerFull);
        return;
      }
    }

    if (m_teams.NumUsed() >= g_app->GetMaxNumberofPlayers() && (g_app->GetMaxNumberofPlayers() != 0))
    {
      // Remove the client
      RemoveClient(_clientId, Disconnect_ServerFull);
      return;
    }
  }
  else if (_teamType == TeamTypeCPU)
  {
    // If the server is full then reject
    if (m_teams.NumUsed() >= g_app->GetMaxNumberofPlayers())
      return;
  }
  else
  {
    // Check if this player is already here
    for (int i = 0; i < m_teams.Size(); ++i)
    {
      if (m_teams.ValidIndex(i))
      {
        ServerTeam* team = m_teams[i];

        if (team->m_clientId == _clientId)
        {
          DebugTrace("SERVER: Failed to create new team - this client already has a Team\n");
          return;
        }
      }
    }

    // If single player or the server is full then reject
    if (g_app->m_clientToServer->GetServerPort() == -1 || (m_teams.NumUsed() >= g_app->GetMaxNumberofPlayers() && (g_app->
      GetMaxNumberofPlayers() != 0)))
    {
      // Remove the client
      RemoveClient(_clientId, Disconnect_ServerFull);
      return;
    }
  }

  DEBUG_ASSERT(m_teams.NumUsed() < NUM_TEAMS);
  auto team = new ServerTeam(_clientId, _teamType);
  int teamId = m_teams.PutData(team);

  auto letter = new ServerToClientLetter();

  letter->m_data = new Directory();
  letter->m_data->SetName(NET_DARWINIA_MESSAGE);
  letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_TEAMASSIGN);
  letter->m_data->CreateData(NET_DARWINIA_TEAMID, teamId);
  letter->m_data->CreateData(NET_DARWINIA_TEAMTYPE, _teamType);
  letter->m_data->CreateData(NET_DARWINIA_CLIENTID, _clientId);

  SendLetter(letter);
}

Directory* Server::GetNextLetter()
{
  m_inboxMutex->Lock();
  Directory* letter = nullptr;

  if (m_inbox.Size() > 0)
  {
    letter = m_inbox[0];
    m_inbox.RemoveData(0);
  }

  m_inboxMutex->Unlock();
  return letter;
}

void Server::ReceiveLetter(Directory* update, const char* fromIP, int _fromPort, int _bandwidthUsed)
{
  m_receiveRate.Count(_bandwidthUsed);

  update->CreateData(NET_DARWINIA_FROMIP, fromIP);
  update->CreateData(NET_DARWINIA_FROMPORT, _fromPort);

  m_inboxMutex->Lock();
  m_inbox.PutDataAtEnd(update);
  m_inboxMutex->Unlock();
}

void Server::SendLetter(ServerToClientLetter* letter)
{
  if (letter)
  {
    //
    // Assign a sequence id

    letter->m_data->CreateData(NET_DARWINIA_SEQID, m_sequenceId);
    m_sequenceId++;

    m_history.PutDataAtEnd(letter);
  }
}

// To handle waking from sleep for instance
void Server::SkipToTime(double _timeNow) { m_nextServerAdvanceTime = _timeNow; }

void Server::AdvanceIfNecessary(double _timeNow)
{
  if (_timeNow > m_nextServerAdvanceTime)
  {
    g_app->m_server->Advance();
    m_nextServerAdvanceTime += 0.1f;
  }
}

void Server::AdvanceSender()
{
  static int s_largest = 0;

  m_outboxMutex->Lock();

  while (m_outbox.Size())
  {
    ServerToClientLetter* letter = m_outbox[0];
    DEBUG_ASSERT(letter);

    letter->m_data->CreateData(NET_DARWINIA_LASTSEQID, m_sequenceId);

    // Send directly to locally connected client without passing through
    // the socket system
    if (g_app->m_clientToServer->m_clientId == letter->m_receiverId)
    {
      g_app->m_clientToServer->ReceiveLetter(letter->m_data);
      letter->m_data = nullptr;
      delete letter;
    }
    else
    {
      int linearSize = 0;
      ServerToClient* client = GetClient(letter->m_receiverId);

      if (client == nullptr)
      {
        DebugTrace("Client is null. Checking disconnected client list\n");
        client = GetDisconnectedClient(letter->m_receiverId);
        DebugTrace("Client was %sin disconnected client list\n", client == nullptr ? "not " : "");
      }

      if (client != nullptr)
      {
        int seqId = letter->m_data->GetDataInt(NET_DARWINIA_SEQID);
        auto cmd = "";

        Directory* firstSubDir = letter->m_data->GetDirectory(NET_DARWINIA_MESSAGE);
        if (firstSubDir)
          cmd = firstSubDir->GetDataString(NET_DARWINIA_COMMAND);

        char* linearisedLetter = letter->m_data->Write(linearSize);

        NetSocketSession* socket = client->GetSocket();
        socket->WriteData(linearisedLetter, linearSize);

        unsigned totalSize = linearSize + UDP_HEADER_SIZE;
        m_sendRate.Count(totalSize);

        double now = GetHighResTime();
        static double lastPrintTime = now;

        if (linearSize > s_largest)
        {
          s_largest = totalSize;
          DebugTrace("SERVER: Largest datagram sent : %d bytes\n", s_largest);
        }
        else if (now > lastPrintTime + 5)
        {
          lastPrintTime = now;
#ifdef _DEBUG
          DebugTrace("server: sending datagram of %d bytes\n", totalSize);
#endif

        }

        delete letter;
        delete [] linearisedLetter;
      }
    }

    // The letter has now been sent so we can take it off the outbox list
    m_outbox.RemoveData(0);
  }

  m_outboxMutex->Unlock();
}

void Server::NotifyNetSyncError(int _clientId, int _sequenceId)
{
  char syncErrorId[256];
  time_t theTimeT = time(nullptr);
  tm* theTime = localtime(&theTimeT);
  sprintf(syncErrorId, "%02d-%02d-%02d %d.%02d.%d %d", 1900 + theTime->tm_year, theTime->tm_mon + 1, theTime->tm_mday, theTime->tm_hour,
          theTime->tm_min, theTime->tm_sec, _sequenceId);

  auto letter = new ServerToClientLetter();
  letter->m_data = new Directory();
  letter->m_data->SetName(NET_DARWINIA_MESSAGE);
  letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_NETSYNCERROR);
  letter->m_data->CreateData(NET_DARWINIA_CLIENTID, _clientId);
  letter->m_data->CreateData(NET_DARWINIA_SYNCERRORID, syncErrorId);
  letter->m_data->CreateData(NET_DARWINIA_SYNCERRORSEQID, _sequenceId);
  SendLetter(letter);

  DebugTrace("SYNCERROR Server: Notified all clients they are out of sync\n");
}

void Server::NotifyNetSyncFixed(int _clientId)
{
  auto letter = new ServerToClientLetter();
  letter->m_data = new Directory();
  letter->m_data->SetName(NET_DARWINIA_MESSAGE);
  letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_NETSYNCFIXED);
  letter->m_data->CreateData(NET_DARWINIA_CLIENTID, _clientId);
  SendLetter(letter);

  DebugTrace("SYNCFIXED Server: Notified client %d his Sync Error is now corrected\n", _clientId);
}

static bool SaveLast(const char* _incomingCmd, const char* _cmd, Directory*& _last, Directory*& _incoming)
{
  if (strcmp(_incomingCmd, _cmd) == 0)
  {
    delete _last;
    _last = _incoming;
    _incoming = nullptr;
    return true;
  }
  return false;
}

static void AddLast(ServerToClientLetter* _letter, Directory*& _msg)
{
  if (_msg)
  {
    _letter->AddUpdate(_msg);
    delete _msg;
    _msg = nullptr;
  }
}

static bool SecondMessage(const char* _incomingCmd, const char* _cmd, bool& _messageAlreadyReceived, Directory*& _incoming)
{
  if (strcmp(_incomingCmd, _cmd) == 0)
  {
    if (_messageAlreadyReceived)
    {
      delete _incoming;
      _incoming = nullptr;
      return true;
    }
    _messageAlreadyReceived = true;
  }
  return false;
}

void Server::Advance()
{
  START_PROFILE("Advance Server");

  //
  // Compile all incoming messages into a ServerToClientLetter

  auto letter = new ServerToClientLetter();

  letter->m_data = new Directory();
  letter->m_data->SetName(NET_DARWINIA_MESSAGE);
  letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_UPDATE);

  while (true)
  {
    Directory* incoming = GetNextLetter();

    if (!incoming)
      break;

    //
    // Sanity check the message

    if (strcmp(incoming->m_name, NET_DARWINIA_MESSAGE) != 0 || !incoming->HasData(NET_DARWINIA_COMMAND, DIRECTORY_TYPE_STRING))
    {
      DebugTrace("Server received bogus message, discarded (12)\n");
      delete incoming;
      continue;
    }

    char* cmd = incoming->GetDataString(NET_DARWINIA_COMMAND);
    int lastSeqId = incoming->GetDataInt(NET_DARWINIA_LASTSEQID);

    char fromStr[64];

    char* fromIp = incoming->GetDataString(NET_DARWINIA_FROMIP);
    int fromPort = incoming->GetDataInt(NET_DARWINIA_FROMPORT);
    int clientId = GetClientId(fromIp, fromPort);

    sprintf(fromStr, "%s:%d", fromIp, fromPort);
    bool isDisconnectedClient = IsDisconnectedClient(fromIp, fromPort);
    if (isDisconnectedClient && !strcmp(cmd, NET_DARWINIA_CLIENT_JOIN) == 0)
    {
      DebugTrace("Disconnected client message from %s\n", fromStr);
      bool handled = HandleDisconnectedClient(incoming);
      DebugTrace("We %s disconnected client\n", handled ? "handled" : "did not handle");
    }
    else if (strcmp(cmd, NET_DARWINIA_CLIENT_JOIN) == 0)
    {
      bool rejectJoin = false;

      if (clientId == -1)
      {
        RemoveDisconnectedClient(fromIp, fromPort);

        DebugTrace("SERVER: New Client connected from {}\n", fromStr);
        clientId = RegisterNewClient(incoming);

        SendClientId(clientId);
      }
    }
    else if (strcmp(cmd, NET_DARWINIA_CLIENT_LEAVE) == 0)
    {
      if (clientId != -1)
      {
        DebugTrace("SERVER: Client at {} disconnected gracefully\n", fromStr);
        RemoveClient(clientId, Disconnect_ClientLeave);
      }
    }
    else if (strcmp(cmd, NET_DARWINIA_REQUEST_TEAM) == 0)
    {
      if (clientId != -1 /* && !g_app->m_gameRunning */)
      {
        DebugTrace("SERVER: New team request from {}\n", fromStr);
        int teamType = incoming->GetDataInt(NET_DARWINIA_TEAMTYPE);
        int desiredTeamId = incoming->GetDataInt(NET_DARWINIA_DESIREDTEAMID);
        RegisterNewTeam(clientId, teamType, desiredTeamId);
      }
    }
    else if (strcmp(cmd, NET_DARWINIA_REQUEST_SPECTATOR) == 0)
    {
      unsigned char teamId = incoming->GetDataUChar(NET_DARWINIA_TEAMID);
      unsigned char clientId = incoming->GetDataUChar(NET_DARWINIA_CLIENTID);

      // BYRON TODO: Explain rationale here
      for (int i = 0; i < m_teams.Size(); i++)
      {
        if (m_teams.ValidIndex(i))
        {
          ServerTeam* pTeam = m_teams[i];

          if (pTeam->m_clientId == static_cast<int>(clientId))
          {
            m_teams.RemoveData(i);
            break;
          }
        }
      }

      // If single player then reject
      if (g_app->m_clientToServer->GetServerPort() == -1)
      {
        // Remove the client
        RemoveClient(clientId, Disconnect_ServerFull);
        return;
      }

      // BYRON TODO: Move this into ClientToServer::RequestSpectator method
      // Send a assign letter
      auto letter = new Directory();
      letter->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_ASSIGN_SPECTATOR);
      letter->CreateData(NET_DARWINIA_TEAMID, teamId);
      letter->CreateData(NET_DARWINIA_CLIENTID, clientId);
      g_app->m_clientToServer->SendLetter(letter);
    }
    else if (strcmp(cmd, NET_DARWINIA_SYNCHRONISE) == 0)
    {
      if (clientId != -1)
        HandleSyncMessage(incoming, clientId);
    }
    else if (strcmp(cmd, NET_DARWINIA_FILESEND) == 0)
    {
      if (clientId != -1)
      {
        ServerToClient* sToC = GetClient(clientId);

        FTPManager* ftpManager = sToC->GetFTPManager();
        ftpManager->ReceiveLetter(incoming);
      }
    }
    else
    {
      // TO DO: See Defcon for special case on removing AI team

      int teamId = incoming->GetDataUChar(NET_DARWINIA_TEAMID);

      if (teamId != 255 && clientId != -1)
      {
        ServerToClient* sToC = GetClient(clientId);

        // We only keep the last ALIVE message from the clients. This prevents a 
        // growing snowball of ALIVE messages if the server starts to fall behind 
        // the update rate for whatever reason.
        //
        // Similary, we only keep the last NET_DARWINIA_SELECTUNIT as these can be 
        // sent multiple times within a session.

        if (!SaveLast(cmd, NET_DARWINIA_ALIVE, sToC->m_lastAlive, incoming) && !
          SaveLast(cmd, NET_DARWINIA_SELECTUNIT, sToC->m_lastSelectUnit, incoming) && !SaveLast(
            cmd, NET_DARWINIA_REQUEST_COLOUR, sToC->m_lastTeamColour, incoming) && !SecondMessage(
            cmd, NET_DARWINIA_START_GAME, m_gameStartMsgReceived, incoming))
          letter->AddUpdate(incoming);
      }
    }

    if (clientId != -1)
    {
      ServerToClient* sToc = GetClient(clientId);

      if (sToc)
      {
        // It could be that the client has disconnected, in which 
        // case he won't be listed in our clients list anymore.

        sToc->m_lastMessageReceived = GetHighResTime();
        if (lastSeqId > sToc->m_lastKnownSequenceId)
          sToc->m_lastKnownSequenceId = lastSeqId;
      }
    }

    delete incoming;
    incoming = nullptr;
  }

  // Add in Alive messages
  int clientsSize = m_clients.Size();
  for (int i = 0; i < clientsSize; ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* sToC = m_clients[i];

      AddLast(letter, sToC->m_lastAlive);
      AddLast(letter, sToC->m_lastSelectUnit);
      AddLast(letter, sToC->m_lastTeamColour);
    }
  }

  SendLetter(letter);

  //
  // Update all clients by sending the next updates to them

  START_PROFILE("UpdateClients");
  UpdateClients();
  END_PROFILE("UpdateClients");

  //
  // Authenticate all connected clients
  // Check for duplicate key usage
  // Kick clients who are using dodgy keys

  START_PROFILE("Authentication");
  AuthenticateClients();
  END_PROFILE("Authentication");

  START_PROFILE("SEND");
  AdvanceSender();
  END_PROFILE("SEND");

  //
  // Advertise our existence

  START_PROFILE("Advertise");
  if (!m_noAdvertise)
    Advertise();
  END_PROFILE("Advertise");

  END_PROFILE("Advance Server");
}

int Server::CountEmptyMessages(int _startingSeqId)
{
  int result = 0;

  for (int i = _startingSeqId; i < m_history.Size(); ++i)
  {
    if (m_history.ValidIndex(i))
    {
      ServerToClientLetter* theLetter = m_history[i];
      char* cmd = theLetter->m_data->GetDataString(NET_DARWINIA_COMMAND);
      if (strcmp(cmd, NET_DARWINIA_UPDATE) == 0 && theLetter->m_data->m_subDirectories.NumUsed() == 0)
        result++;
      else
        break;
    }
  }

  return result;
}

void Server::AuthenticateClients()
{
  for (int i = 0; i < m_clients.Size(); ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* s2c = m_clients[i];

      // Do a basic check
      if (s2c->m_basicAuthCheck == 0)
        AuthenticateClient(s2c->m_clientId);

      // BYRON TODO - re-enabled this check. I had to remove it because it was stopping me from testing
      // Do a key check via the metaserver
      int keyResult = Authentication_GetStatus(s2c->m_authKey);
      if (keyResult < 0)
        s2c->m_basicAuthCheck = -3;

      // If its bad kick them now
      if (s2c->m_basicAuthCheck < 0)
      {
#if AUTHENTICATION_LEVEL == 1
        int kickReason = (s2c->m_basicAuthCheck == -1
          ? Disconnect_InvalidKey
          : s2c->m_basicAuthCheck == -2
          ? Disconnect_DuplicateKey
          : s2c->m_basicAuthCheck == -3
          ? Disconnect_KeyAuthFailed
          : s2c->m_basicAuthCheck == -4
          ? Disconnect_BadPassword
          : s2c->m_basicAuthCheck == -5
          ? Disconnect_ServerFull
          : s2c->m_basicAuthCheck == -6
          ? Disconnect_DemoFull
          : Disconnect_InvalidKey);

        if (g_app->m_clientToServer->GetServerPort() != -1)
        {
          // dont do this in single player
          RemoveClient(s2c->m_clientId, kickReason);
        }
#endif
      }
    }
  }
}

void Server::AuthenticateClient(int _clientId)
{
  ServerToClient* client = GetClient(_clientId);
  ASSERT(client);

  int numTeams = m_teams.NumUsed();
  int maxTeams = g_app->GetMaxNumberofPlayers();

  if (maxTeams != 0 && numTeams != 0)
  {
    if (numTeams >= maxTeams)
    {
      DebugTrace("Teams are already full, client %d cannot join\n", client->m_clientId);
      client->m_basicAuthCheck = -5;
      return;
    }
  }

  if (m_serverPassword.Length() > 0)
  {
    if (client->m_password != m_serverPassword)
    {
      client->m_basicAuthCheck = -4;
      DebugTrace("Client has failed the password auth check\n");
      return;
    }
  }

  // Defcon also checked:
  //
  // Are there too many teams already?
  //
  // Are there too many DEMO teams?
  // Or is this an unusual game mode?
  // Only affects this client if they are themselves a demo
  //
  // If we are running a MOD and the client does not support MODs
  // (ie < v1.2) we must disconnect them now.
  //
  // Check for a server password
  //
  // Everything looks good

  client->m_basicAuthCheck = 1;
}

void Server::UpdateClients()
{
  // LEANDER : PC build disconnected clients garbage collection 
  int clientsSize = m_clients.Size();
  for (int i = 0; i < clientsSize; ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* s2c = m_clients[i];

      double timeNow = GetHighResTime();
      double timeSinceLastMessage = timeNow - s2c->m_lastMessageReceived;

#ifndef _DEBUG
      float maxTimeout = 20.0;

      if (timeSinceLastMessage > maxTimeout)
      {
        // This person just isn't responding anymore
        // So disconnect them
        RemoveClient(s2c->m_clientId, Disconnect_ClientDrop);
        continue;
      }
#endif

      s2c->m_lastSentSequenceId = max(s2c->m_lastSentSequenceId, s2c->m_lastKnownSequenceId);

      int sendFrom = s2c->m_lastSentSequenceId + 1;
      int sendTo = m_history.Size();
      if (sendTo - sendFrom > 3)
        sendTo = sendFrom + 3;

      int fallenBehindThreshold = 10;

      if (s2c->m_lastKnownSequenceId < m_history.Size() - fallenBehindThreshold)
      {
        // This client appears to have lost some packets and fallen behind, so rewind a bit
        s2c->m_caughtUp = false;
      }
      else if (s2c->m_lastKnownSequenceId > m_history.Size() - fallenBehindThreshold / 2 - 1)
        s2c->m_caughtUp = true;

      if (!s2c->m_caughtUp)
      {
        if (s2c->m_lastKnownSequenceId < s2c->m_lastSentSequenceId - fallenBehindThreshold)
          sendFrom = s2c->m_lastKnownSequenceId + 1;

        sendTo = sendFrom + 3;
      }

      //
      // Special case - do run-length style encoding if there are lots of empty messages

      int numEmptyMessages = CountEmptyMessages(sendFrom);
      if (numEmptyMessages > 5 && timeSinceLastMessage < 5.0)
      {
        auto letter = new ServerToClientLetter();
        letter->m_receiverId = s2c->m_clientId;
        letter->m_data = new Directory();
        letter->m_data->SetName(NET_DARWINIA_MESSAGE);
        letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_UPDATE);
        letter->m_data->CreateData(NET_DARWINIA_NUMEMPTYUPDATES, numEmptyMessages);
        letter->m_data->CreateData(NET_DARWINIA_SEQID, sendFrom);

        m_outboxMutex->Lock();
        m_outbox.PutDataAtEnd(letter);
        m_outboxMutex->Unlock();

        s2c->m_lastSentSequenceId = sendFrom + numEmptyMessages - 1;
      }
      else
      {
        for (int l = sendFrom; l < sendTo; ++l)
        {
          if (m_history.ValidIndex(l))
          {
            ServerToClientLetter* theLetter = m_history[l];
            if (theLetter)
            {
              auto letterCopy = new ServerToClientLetter();
              letterCopy->m_data = new Directory(*theLetter->m_data);
              letterCopy->m_receiverId = s2c->m_clientId;

              // To help combat packet loss, re-send messages that have definately been sent
              // (ie < lastSentSequenceId) but havent yet been acknowledged (>lastKnownSequenceId)

              int historyIndex = s2c->m_lastKnownSequenceId + (l - sendFrom) + 1;

              if (!s2c->m_caughtUp)
              {
                // This client is having problems.  Assume he may have lost
                // some messages, even though he told us he received them.
                historyIndex -= 9;
              }

              if (historyIndex < l && m_history.ValidIndex(historyIndex))
              {
                auto prevData = new Directory(*m_history[historyIndex]->m_data);
                prevData->SetName(NET_DARWINIA_PREVUPDATE);
                letterCopy->m_data->AddDirectory(prevData);
              }

              m_outboxMutex->Lock();
              m_outbox.PutDataAtEnd(letterCopy);
              m_outboxMutex->Unlock();

              s2c->m_lastSentSequenceId = l;
            }
          }
        }
      }

      //
      // FTPManager
      FTPManager* ftpManager = s2c->m_ftpManager;
      if (ftpManager)
      {
        LList<Directory*> ftpMessages;

        ftpManager->MakeSendLetters(ftpMessages);

        int size = ftpMessages.Size();

        m_outboxMutex->Lock();
        for (int i = 0; i < size; i++)
        {
          auto s = new ServerToClientLetter;
          s->m_data = ftpMessages.GetData(i);
          s->m_receiverId = s2c->m_clientId;

          m_outbox.PutDataAtEnd(s);
        }
        m_outboxMutex->Unlock();
      }

      //
      // Check to see if any clients have sent us IFrame from a sync error
      // If so, send that IFrame to the other clients

      if (ftpManager && s2c->m_syncErrorSeqId != -1)
      {
        char filename[256];
        sprintf(filename, "IFrame %d %d", s2c->m_clientId, s2c->m_syncErrorSeqId);

        FTP* ftp = ftpManager->Retrieve(filename);

        if (ftp)
        {
          // Send the IFrame to the other clients
          for (int j = 0; j < clientsSize; j++)
          {
            if (j != i && m_clients.ValidIndex(j))
            {
              ServerToClient* other = m_clients[j];
              FTPManager* otherFTP = other->GetFTPManager();
              otherFTP->SendFile(filename, ftp->Data(), ftp->Size());
            }
          }

          delete ftp;
        }
      }
    }
  }
}

void Server::HandleSyncMessage(Directory* incoming, int _clientId)
{
  int sequenceId = incoming->GetDataInt(NET_DARWINIA_LASTPROCESSEDSEQID);
  unsigned char sync = incoming->GetDataUChar(NET_DARWINIA_SYNCVALUE);

  if (!m_history.ValidIndex(sequenceId))
  {
    DebugTrace("Invalid SequenceID received! SeqID is %d, clientID is %d, sync char is %d\n", sequenceId, _clientId, sync);
    DEBUG_ASSERT(m_history.ValidIndex(sequenceId));
    return;
  }

  ServerToClient* sToc = GetClient(_clientId);

  //
  // Log the incoming sync value

  if (sToc)
    sToc->m_sync.PutData(sync, sequenceId);

  //
  // Provisional test - is the frame out of sync?

  int numResults = 0;
  bool provisionalResult = true;

  for (int i = 0; i < m_clients.Size(); ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* thisClient = m_clients[i];
      if (thisClient->m_sync.ValidIndex(sequenceId))
      {
        ++numResults;

        unsigned char thisSync = thisClient->m_sync[sequenceId];
        if (thisSync != sync)
          provisionalResult = false;
      }
    }
  }

  //
  // If something went wrong then figure out who is to blame
  // We must have all Sync bytes before we can do this
  // Look for the client with the sync byte that doesn't match anybody else

  bool allResponses = (numResults == m_clients.NumUsed());

  if (allResponses && provisionalResult == false)
  {
    for (int i = 0; i < m_clients.Size(); ++i)
    {
      if (m_clients.ValidIndex(i))
      {
        ServerToClient* clientA = m_clients[i];
        if (clientA->m_syncErrorSeqId == -1)
        {
          unsigned char clientASync = clientA->m_sync[sequenceId];
          int numSame = 0;
          for (int j = 0; j < m_clients.Size(); ++j)
          {
            if (i != j && m_clients.ValidIndex(j))
            {
              ServerToClient* clientB = m_clients[j];
              if (clientB->m_syncErrorSeqId == -1 || clientB->m_syncErrorSeqId > sequenceId)
              {
                unsigned char clientBSync = clientB->m_sync[sequenceId];
                if (clientASync == clientBSync)
                  ++numSame;
              }
            }
          }

          if (numSame == 0)
          {
            clientA->m_syncErrorSeqId = sequenceId;
            NotifyNetSyncError(i, sequenceId);
          }
        }
      }
    }
  }
}

bool Server::IsDisconnectedClient(const char* _ip, int _port)
{
  for (int i = 0; i < m_disconnectedClients.Size(); ++i)
  {
    if (m_disconnectedClients.ValidIndex(i))
    {
      ServerToClient* sToC = m_disconnectedClients[i];
      if (strcmp(sToC->m_ip, _ip) == 0 && sToC->m_port == _port)
        return true;
    }
  }

  return false;
}

bool Server::HandleDisconnectedClient(Directory* _message)
{
  char* fromIp = _message->GetDataString(NET_DARWINIA_FROMIP);
  int fromPort = _message->GetDataInt(NET_DARWINIA_FROMPORT);
  ServerToClient* sToC = GetDisconnectedClient(fromIp, fromPort);

  if (!sToC)
  {
    DebugTrace("Could not find this disconnected client's ServerToClient\n");
    return false;
  }

  auto letter = new ServerToClientLetter();
  letter->m_data = new Directory();
  letter->m_data->SetName(NET_DARWINIA_MESSAGE);
  letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_DISCONNECT);
  letter->m_data->CreateData(NET_DARWINIA_DISCONNECT, sToC->m_disconnected);
  letter->m_receiverId = sToC->m_clientId;
  letter->m_data->CreateData(NET_DARWINIA_SEQID, -1);
  letter->m_clientDisconnected = true;

  m_outbox.PutDataAtEnd(letter);

  sToC->m_lastMessageReceived = GetHighResTime();

  char fromStr[64];
  sprintf(fromStr, "%s:%d", sToC->m_ip, sToC->m_port);

  DebugTrace("Re-sent disconnect message to client {}\n", fromStr);

  return true;
}

ServerToClient* Server::GetDisconnectedClient(int _id)
{
  for (int i = 0; i < m_disconnectedClients.Size(); ++i)
  {
    if (m_disconnectedClients.ValidIndex(i))
    {
      ServerToClient* sToC = m_disconnectedClients[i];
      if (sToC->m_clientId == _id)
        return sToC;
    }
  }

  return nullptr;
}

ServerToClient* Server::GetDisconnectedClient(const char* _ip, int _port)
{
  for (int i = 0; i < m_disconnectedClients.Size(); ++i)
  {
    if (m_disconnectedClients.ValidIndex(i))
    {
      ServerToClient* sToC = m_disconnectedClients[i];
      if (strcmp(sToC->m_ip, _ip) == 0 && sToC->m_port == _port)
        return sToC;
    }
  }

  return nullptr;
}

bool Server::RemoveDisconnectedClient(const char* _ip, int _port)
{
  for (int i = m_disconnectedClients.Size() - 1; i >= 0; --i)
  {
    if (m_disconnectedClients.ValidIndex(i))
    {
      ServerToClient* sToC = m_disconnectedClients[i];
      if (strcmp(sToC->m_ip, _ip) == 0 && sToC->m_port == _port)
      {
        m_disconnectedClients.RemoveData(i);
        delete sToC;
      }
    }
  }

  return NULL;
}

void Server::Advertise()
{
  if (strcmp(g_app->m_requestedMap, "null") == 0 || strlen(g_app->m_requestedMap) == 0)
  {
    // no map, dont advertise
    return;
  }

  // WAN / LAN advertisement is a game property.
  bool advertiseOnWan = true, advertiseOnLan = true;

  int maxTeams = 4; // maxTeams is a game property.
  int currentTeams = m_teams.NumUsed();

  //
  // Update our information

  if (advertiseOnWan || advertiseOnLan)
  {
    // Multiwinia is a game property

    const char* defaultServerName = g_prefsManager->GetString("PlayerName", "NewServer"); //getenv("USERNAME");
    if (!defaultServerName)
      defaultServerName = "Multiwinia";

    const char* serverName = g_prefsManager->GetString("ServerName", defaultServerName);

    Directory serverProperties;

    //
    // Basic variables

    serverProperties.CreateData(NET_METASERVER_SERVERNAME, serverName);

    serverProperties.CreateData(NET_METASERVER_GAMENAME, APP_NAME);
    serverProperties.CreateData(NET_METASERVER_GAMEVERSION, APP_VERSION);

    serverProperties.CreateData(NET_METASERVER_STARTTIME, static_cast<unsigned long long>(m_startTimeActual));
    serverProperties.CreateData(NET_METASERVER_RANDOM, static_cast<int>(m_random));

    char localIp[256];
    GetLocalHostIP(localIp, 256);
    int localPort = GetLocalPort();

    serverProperties.CreateData(NET_METASERVER_LOCALIP, localIp);
    serverProperties.CreateData(NET_METASERVER_LOCALPORT, localPort);

    //
    // Game properties

    int gameType = g_app->m_multiwinia->m_gameType;
    int maxTeams = 1;
    int mapCRC = 0;

    MapData* mapData = nullptr;

    // Determine how many players can join this game
    if (0 <= gameType && gameType < MAX_GAME_TYPES && g_app->m_requestedMap)
    {
      DArray<MapData*>& maps = g_app->m_gameMenu->m_maps[gameType];

      for (int i = 0; i < maps.Size(); i++)
      {
        if (maps.ValidIndex(i))
        {
          MapData* m = maps[i];
          if (stricmp(g_app->m_requestedMap, m->m_fileName) == 0)
          {
            mapData = m;
            maxTeams = m->m_numPlayers;
            mapCRC = m->m_mapId;
            break;
          }
        }
      }
    }

    int currentHumanTeams = m_clients.NumUsed();

    serverProperties.CreateData(NET_METASERVER_NUMTEAMS, static_cast<unsigned char>(currentTeams));
    serverProperties.CreateData(NET_METASERVER_MAXTEAMS, static_cast<unsigned char>(maxTeams));
    serverProperties.CreateData(NET_METASERVER_NUMHUMANTEAMS, static_cast<unsigned char>(currentHumanTeams));
    serverProperties.CreateData(NET_METASERVER_NUMSPECTATORS, static_cast<unsigned char>(0));
    serverProperties.CreateData(NET_METASERVER_MAXSPECTATORS, static_cast<unsigned char>(0));
    serverProperties.CreateData(NET_METASERVER_GAMEINPROGRESS, static_cast<unsigned char>(g_app->m_multiwinia->GameRunning()));
    serverProperties.CreateData(NET_METASERVER_GAMEMODE, static_cast<unsigned char>(gameType));

    if (m_serverPassword.Length() > 0)
      serverProperties.CreateData(NET_METASERVER_HASPASSWORD, static_cast<unsigned char>(1));

    serverProperties.CreateData(NET_METASERVER_MAPCRC, mapCRC);

    char authKey[256];
    Authentication_GetKey(authKey);
    serverProperties.CreateData(NET_METASERVER_AUTHKEY, authKey);

    // LAN advertisements
    serverProperties.CreateData(NET_METASERVER_PORT, localPort);
  }
}

int Server::GetLocalPort()
{
  if (!m_listener)
    return -1;

  return m_listener->GetPort();
}

// ServerToClientLetter

void ServerToClientLetter::AddUpdate(const Directory* _update)
{
  auto copy = new Directory(*_update);
  copy->RemoveData(NET_DARWINIA_FROMIP);
  copy->RemoveData(NET_DARWINIA_FROMPORT);
  copy->RemoveData(NET_DARWINIA_LASTSEQID);

  m_data->AddDirectory(copy);
}

void Server::RegisterSpectator(int _clientId)
{
  ServerToClient* sToC = GetClient(_clientId);
  ASSERT(sToC);

  if (!sToC->m_spectator)
  {
    sToC->m_spectator = true;

    //
    // If he has any (non-AI) teams remove them now

    for (int t = 0; t < m_teams.Size(); ++t)
    {
      if (m_teams.ValidIndex(t))
      {
        ServerTeam* team = m_teams[t];
        if (team->m_clientId == _clientId)
        {
          m_teams.RemoveData(t);
          delete team;
        }
      }
    }
    //}

    //
    // Send a letter to all Clients

    auto letter = new ServerToClientLetter();
    letter->m_data = new Directory();
    letter->m_data->SetName(NET_DARWINIA_MESSAGE);
    letter->m_data->CreateData(NET_DARWINIA_COMMAND, NET_DARWINIA_SPECTATORASSIGN);
    letter->m_data->CreateData(NET_DARWINIA_CLIENTID, _clientId);
    SendLetter(letter);
  }

}

int Server::GetNumSpectators()
{
  int count = 0;

  for (int i = 0; i < m_clients.Size(); ++i)
  {
    if (m_clients.ValidIndex(i))
    {
      ServerToClient* client = m_clients[i];
      if (client->m_spectator)
        ++count;
    }
  }

  return count;
}

void Server::KickClient(int _clientId)
{
  DebugTrace("Server kicking client id %d\n", _clientId);
  RemoveClient(_clientId, Disconnect_KickedByServer);
}

UnicodeString& Server::GetPassword() { return m_serverPassword; }

void Server::SetPassword(UnicodeString& _password) { m_serverPassword = _password; }
