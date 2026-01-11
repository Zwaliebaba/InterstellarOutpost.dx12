#include "pch.h"
#include "binary_stream_readers.h"

size_t FileCRC(BinaryReader* reader)
{
  size_t hash = 0;
  std::hash<uint8_t> hasher;

  while (reader->IsOpen() && !reader->m_eof)
  {
    hash ^= hasher(reader->ReadU8()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }

  return hash;
}
