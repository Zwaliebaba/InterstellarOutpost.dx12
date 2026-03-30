#include <CppUnitTest.h>
#include "DataWriter.h"
#include "DataReader.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTests
{
  TEST_CLASS(DataWriterReaderRoundTripTests)
  {
  public:
    TEST_METHOD(Int32_RoundTrip)
    {
      Neuron::DataWriter writer;
      writer.WriteInt32(0x12345678);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      Assert::AreEqual(0x12345678, reader.Read<int32_t>());
    }

    TEST_METHOD(UInt16_RoundTrip)
    {
      Neuron::DataWriter writer;
      writer.WriteUInt16(0xABCD);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      Assert::AreEqual(static_cast<uint16_t>(0xABCD), reader.Read<uint16_t>());
    }

    TEST_METHOD(Float_RoundTrip)
    {
      Neuron::DataWriter writer;
      writer.WriteFloat(3.14159f);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      Assert::AreEqual(3.14159f, reader.Read<float>(), 1e-5f);
    }

    TEST_METHOD(String_RoundTrip)
    {
      Neuron::DataWriter writer;
      writer.WriteString("InterstellarOutpost");

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      Assert::AreEqual(std::string("InterstellarOutpost"), reader.ReadString());
    }

    TEST_METHOD(MultipleValues_RoundTripInOrder)
    {
      Neuron::DataWriter writer;
      writer.WriteInt32(1);
      writer.WriteFloat(2.5f);
      writer.WriteByte(0xFF);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      Assert::AreEqual(1, reader.Read<int32_t>());
      Assert::AreEqual(2.5f, reader.Read<float>(), 1e-5f);
      Assert::AreEqual(static_cast<uint8_t>(0xFF), reader.Read<uint8_t>());
    }

    TEST_METHOD(Reset_ClearsWrittenData)
    {
      Neuron::DataWriter writer;
      writer.WriteInt32(42);
      writer.Reset();
      Assert::AreEqual(static_cast<size_t>(0), writer.Size());
    }
  };
}
