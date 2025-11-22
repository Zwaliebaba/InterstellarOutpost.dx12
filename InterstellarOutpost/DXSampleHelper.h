#pragma once
#include <stdexcept>

#define SAFE_RELEASE(p) \
  if (p) (p)->Release()

//inline HRESULT ReadDataFromDDSFile(LPCWSTR filename, byte **data, UINT *offset, UINT *size)
//{
//  if (FAILED(ReadDataFromFile(filename, data, size)))
//  {
//    return E_FAIL;
//  }
//
//  // DDS files always start with the same magic number.
//  static const UINT DDS_MAGIC = 0x20534444;
//  UINT magicNumber = *reinterpret_cast<const UINT *>(*data);
//  if (magicNumber != DDS_MAGIC)
//  {
//    return E_FAIL;
//  }
//
//  struct DDS_PIXELFORMAT
//  {
//    UINT size;
//    UINT flags;
//    UINT fourCC;
//    UINT rgbBitCount;
//    UINT rBitMask;
//    UINT gBitMask;
//    UINT bBitMask;
//    UINT aBitMask;
//  };
//
//  struct DDS_HEADER
//  {
//    UINT size;
//    UINT flags;
//    UINT height;
//    UINT width;
//    UINT pitchOrLinearSize;
//    UINT depth;
//    UINT mipMapCount;
//    UINT reserved1[11];
//    DDS_PIXELFORMAT ddsPixelFormat;
//    UINT caps;
//    UINT caps2;
//    UINT caps3;
//    UINT caps4;
//    UINT reserved2;
//  };
//
//  auto ddsHeader = reinterpret_cast<const DDS_HEADER *>(*data + sizeof(UINT));
//  if (ddsHeader->size != sizeof(DDS_HEADER) || ddsHeader->ddsPixelFormat.size != sizeof(DDS_PIXELFORMAT))
//  {
//    return E_FAIL;
//  }
//
//  const ptrdiff_t ddsDataOffset = sizeof(UINT) + sizeof(DDS_HEADER);
//  *offset = ddsDataOffset;
//  *size = *size - ddsDataOffset;
//
//  return S_OK;
//}

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


// Resets all elements in a unique_ptr array.
template<class T>
void ResetUniquePtrArray(T *uniquePtrArray)
{
  for (auto &i: *uniquePtrArray)
  {
    i.reset();
  }
}
