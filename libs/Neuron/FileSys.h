#pragma once

namespace Neuron
{
  using byte_buffer_t = std::vector<uint8_t>;

  class FileSys
  {
    public:
      static void SetHomeDirectory(const std::wstring& _path) { sm_homeDir = _path + L"\\gamedata\\"; }
      [[nodiscard]] static std::wstring GetHomeDirectory() { return sm_homeDir; }

    protected:
      inline static std::wstring sm_homeDir;
  };

  class BinaryFile : public FileSys
  {
    public:
      [[nodiscard]] static byte_buffer_t Read(const std::wstring& _fileName);
  };

  class TextFile : public FileSys
  {
    public:
      [[nodiscard]] static std::wstring Read(const std::wstring& _fileName);
  };
}
