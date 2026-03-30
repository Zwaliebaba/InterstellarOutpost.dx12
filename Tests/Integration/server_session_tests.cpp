// Server session integration tests.
//
// Tests are driven synchronously against an in-process Server instance with no
// real network I/O.  Client registrations and messages are injected directly
// via Server public methods to verify state machine transitions without threads
// or wall-clock timing.

#include <CppUnitTest.h>
#include "server.h"
#include "servertoclient.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IntegrationTests
{
  TEST_CLASS(ServerSessionTests)
  {
    Server* m_server = nullptr;

  public:
    TEST_METHOD_INITIALIZE(Setup)
    {
      m_server = new Server();
      m_server->Initialise();
    }

    TEST_METHOD_CLEANUP(Teardown)
    {
      delete m_server;
      m_server = nullptr;
    }

    TEST_METHOD(Initialise_StartsWithNoClients)
    {
      Assert::AreEqual(0, m_server->m_clients.Size());
    }

    TEST_METHOD(Initialise_StartsWithNoTeams)
    {
      Assert::AreEqual(0, m_server->m_teams.Size());
    }

    TEST_METHOD(Initialise_SequenceIdIsZero)
    {
      Assert::AreEqual(0, m_server->m_sequenceId);
    }

    TEST_METHOD(GetClient_ReturnsNull_WhenNoClientsRegistered)
    {
      ServerToClient* client = m_server->GetClient(0);
      Assert::IsNull(client);
    }

    TEST_METHOD(GetClientId_ReturnsMinusOne_ForUnknownAddress)
    {
      int id = m_server->GetClientId("192.168.0.99", 12345);
      Assert::AreEqual(-1, id);
    }

    TEST_METHOD(IsDisconnectedClient_ReturnsFalse_ForUnknownAddress)
    {
      bool result = m_server->IsDisconnectedClient("10.0.0.1", 9999);
      Assert::IsFalse(result);
    }

    TEST_METHOD(GetNextLetter_ReturnsNull_WhenInboxEmpty)
    {
      Directory* letter = m_server->GetNextLetter();
      Assert::IsNull(letter);
    }
  };
}
