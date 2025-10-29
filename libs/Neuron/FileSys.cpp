#include "pch.h"
#include "FileSys.h"

namespace
{
  // Common file reading implementation used by both BinaryFile and TextFile
  template <typename T>
  T ReadFileImpl(const std::wstring& fullPath, bool isBinary)
  {
    T data;

    ScopedHandle hFile(safe_handle(CreateFile2(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr)));
    if (!hFile)
    {
      // TODO: Consider adding error logging here
      return {};
    }

    // Get the file size
    FILE_STANDARD_INFO fileInfo;
    if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
    {
      DebugTrace(L"Failed to get file info: {}\n", fullPath);
      return {};
    }

    // File is too big for 32-bit allocation, so reject read
    if (fileInfo.EndOfFile.HighPart > 0)
    {
      DebugTrace(L"File too large to read: {}\n", fullPath);
      return {};
    }

    // Create enough space for the file data
    data.resize(fileInfo.EndOfFile.LowPart);

    // Read the data in
    DWORD bytesRead = 0;
    if (!::ReadFile(hFile.get(), data.data(), fileInfo.EndOfFile.LowPart, &bytesRead, nullptr))
    {
      DebugTrace(L"Failed to read file: {}\n", fullPath);
      return {};
    }

    if (bytesRead < fileInfo.EndOfFile.LowPart)
    {
      DebugTrace(L"Only read {} of {} bytes from file: {}\n", bytesRead, fileInfo.EndOfFile.LowPart, fullPath);
      return {};
    }

    return data;
  }
} // anonymous namespace

byte_buffer_t BinaryFile::Read(const std::wstring& _fileName)
{
  std::wstring fullName = GetHomeDirectory() + _fileName;
  return ReadFileImpl<byte_buffer_t>(fullName, true);
}

std::wstring TextFile::Read(const std::wstring& _fileName)
{
  std::wstring fullName = GetHomeDirectory() + _fileName;
  return ReadFileImpl<std::wstring>(fullName, false);
}
