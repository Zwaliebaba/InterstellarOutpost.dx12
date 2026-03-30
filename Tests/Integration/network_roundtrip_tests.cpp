// Network round-trip integration tests.
//
// These tests exercise DataWriter/DataReader serialisation together with the
// NetLib UDP socket layer using loopback (127.0.0.1).  All networking is
// driven synchronously — no threads, no wall-clock sleeps.

#include <CppUnitTest.h>
#include "net_lib.h"
#include "net_socket_listener.h"
#include "net_socket_session.h"
#include "DataWriter.h"
#include "DataReader.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IntegrationTests
{
  TEST_CLASS(NetworkRoundTripTests)
  {
    static NetLib* s_netLib;

  public:
    TEST_CLASS_INITIALIZE(InitNetLib)
    {
      s_netLib = new NetLib();
      if (!s_netLib->Initialise())
      {
        delete s_netLib;
        s_netLib = nullptr;
        Logger::WriteMessage("NetLib::Initialise failed — network tests skipped.");
      }
    }

    TEST_CLASS_CLEANUP(ShutdownNetLib)
    {
      delete s_netLib;
      s_netLib = nullptr;
    }

    TEST_METHOD(SendAndReceive_Int32_ViaLoopback)
    {
      if (!s_netLib) { Logger::WriteMessage("Skipped: NetLib unavailable."); return; }

      constexpr unsigned short kPort = 27199;

      NetSocketListener listener(kPort);
      NetSocketSession  sender(listener, "127.0.0.1", kPort);

      Neuron::DataWriter writer;
      writer.WriteInt32(0xDEADBEEF);

      NetRetCode rc = sender.WriteData(writer.Data(), static_cast<int>(writer.Size()));
      Assert::AreEqual(static_cast<int>(NetOk), static_cast<int>(rc));

      // Receive
      char buf[MAX_PACKET_SIZE] = {};
      NetIpAddress from{};
      int received = 0;
      rc = listener.ReadData(buf, MAX_PACKET_SIZE, &received, &from);
      Assert::AreEqual(static_cast<int>(NetOk), static_cast<int>(rc));
      Assert::AreEqual(static_cast<int>(sizeof(int32_t)), received);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(buf), received);
      Assert::AreEqual(static_cast<int32_t>(0xDEADBEEF), reader.Read<int32_t>());
    }

    TEST_METHOD(SendAndReceive_MultipleValues_ViaLoopback)
    {
      if (!s_netLib) { Logger::WriteMessage("Skipped: NetLib unavailable."); return; }

      constexpr unsigned short kPort = 27198;

      NetSocketListener listener(kPort);
      NetSocketSession  sender(listener, "127.0.0.1", kPort);

      Neuron::DataWriter writer;
      writer.WriteUInt16(42u);
      writer.WriteFloat(1.618f);
      writer.WriteByte(0xAB);

      sender.WriteData(writer.Data(), static_cast<int>(writer.Size()));

      char buf[MAX_PACKET_SIZE] = {};
      NetIpAddress from{};
      int received = 0;
      listener.ReadData(buf, MAX_PACKET_SIZE, &received, &from);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(buf), received);
      Assert::AreEqual(static_cast<uint16_t>(42u), reader.Read<uint16_t>());
      Assert::AreEqual(1.618f, reader.Read<float>(), 1e-4f);
      Assert::AreEqual(static_cast<uint8_t>(0xAB), reader.Read<uint8_t>());
    }
  };

  NetLib* NetworkRoundTripTests::s_netLib = nullptr;
}
