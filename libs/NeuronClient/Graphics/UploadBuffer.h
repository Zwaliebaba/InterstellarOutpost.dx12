// Description:  An upload buffer is visible to both the CPU and the GPU, but
// because the memory is write combined, you should avoid reading data with the CPU.
// An upload buffer is intended for moving data to a default GPU buffer.  You can
// read from a file directly into an upload buffer, rather than reading into regular
// cached memory, copying that to an upload buffer, and copying that to the GPU.

#pragma once

#include "GpuResource.h"

class UploadBuffer : public GpuResource
{
public:
  virtual ~UploadBuffer() { Destroy(); }

  void Create(const std::wstring &name, size_t BufferSize);

  void *Map(void);
  void Unmap(size_t begin = 0, size_t end = -1);

  size_t GetBufferSize() const { return m_BufferSize; }

protected:
  size_t m_BufferSize;
};
