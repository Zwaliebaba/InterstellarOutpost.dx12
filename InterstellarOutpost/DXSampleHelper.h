#pragma once

#define SAFE_RELEASE(p) \
  if (p) (p)->Release()

inline UINT CalculateConstantBufferByteSize(UINT byteSize)
{
  // Constant buffer size is required to be aligned.
  return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

// Resets all elements in a com_ptr array.
template<class T>
void ResetComPtrArray(T *comPtrArray)
{
  for (auto &i: *comPtrArray)
  {
    i = nullptr;
  }
}
