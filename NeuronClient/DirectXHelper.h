#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#define D3DX12_NO_STATE_OBJECT_HELPERS
#include "d3dx12.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

#define IID_GRAPHICS_PPV_ARGS(ppType)       __uuidof(ppType), (ppType).put_void()

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

constexpr D3D12_CPU_DESCRIPTOR_HANDLE D3D12_CPU_HANDLE_NULL = {D3D12_GPU_VIRTUAL_ADDRESS_NULL};
constexpr D3D12_CPU_DESCRIPTOR_HANDLE D3D12_CPU_HANDLE_UNKNOWN = {D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN};

constexpr D3D12_GPU_DESCRIPTOR_HANDLE D3D12_GPU_HANDLE_NULL = {D3D12_GPU_VIRTUAL_ADDRESS_NULL};
constexpr D3D12_GPU_DESCRIPTOR_HANDLE D3D12_GPU_HANDLE_UNKNOWN = {D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN};

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

consteval uint32_t FIXED_MSG(const char* _msg)
{
  return static_cast<uint32_t>(_msg[0]) << 24 | static_cast<uint32_t>(_msg[1]) << 16 | static_cast<uint32_t>(_msg[2]) << 8 | static_cast<
    uint32_t>(_msg[3]);
}

#if defined _DEBUG
inline void SetName(ID3D12Object* _pObject, const LPCWSTR _name) { check_hresult(_pObject->SetName(_name)); }

inline void SetNameIndexed(ID3D12Object* _pObject, const LPCWSTR _name, const UINT _index)
{
  WCHAR fullName[50];
  if (swprintf_s(fullName, L"%s[%u]", _name, _index) > 0)
    check_hresult(_pObject->SetName(fullName));
}

#else
inline void SetName(ID3D12Object* _pObject, const LPCWSTR _name) {}inline void SetNameIndexed(
  ID3D12Object* _pObject, const LPCWSTR _name, const UINT _index) {}
#endif

#define NAME_D3D12_OBJECT(x) SetName((x).get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].get(), L#x, n)

class GpuUploadBuffer
{
  public:
    com_ptr<ID3D12Resource> GetResource() { return m_resource; }
    virtual void Release() { m_resource = nullptr; }
    UINT64 Size() { return m_resource.get() ? m_resource->GetDesc().Width : 0; }

  protected:
    com_ptr<ID3D12Resource> m_resource;

    GpuUploadBuffer() = default;

    ~GpuUploadBuffer()
    {
      if (m_resource.get())
        m_resource->Unmap(0, nullptr);
    }

    void Allocate(ID3D12Device5* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
    {
      auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

      auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
      check_hresult(device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(m_resource)));
      if (resourceName)
        m_resource->SetName(resourceName);
    }

    uint8_t* MapCpuWriteOnly() const
    {
      uint8_t* mappedData;
      // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
      CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
      check_hresult(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
      return mappedData;
    }
};

// Helper class to create and update a constant buffer with proper constant buffer alignments.
// Usage: 
//    ConstantBuffer<...> cb;
//    cb.Create(...);
//    cb.staging.var = ... ; | cb->var = ... ; 
//    cb.CopyStagingToGPU(...);
//    Set...View(..., cb.GputVirtualAddress());
template <class T>
class ConstantBuffer : public GpuUploadBuffer
{
  uint8_t* m_mappedConstantData;
  size_t m_alignedInstanceSize;
  size_t m_numInstances;

  public:
    ConstantBuffer()
      : m_mappedConstantData(nullptr),
        m_alignedInstanceSize(0),
        m_numInstances(0) {}

    virtual ~ConstantBuffer() = default;

    void Create(ID3D12Device5* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
      m_numInstances = numInstances;
      m_alignedInstanceSize = AlignUp(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
      size_t bufferSize = numInstances * m_alignedInstanceSize;
      Allocate(device, bufferSize, resourceName);
      m_mappedConstantData = MapCpuWriteOnly();
    }

    void CopyStagingToGpu(size_t instanceIndex = 0)
    {
      memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
    }

    // Accessors
    // Align staging object on 16B boundary for faster mempcy to the memory returned by Map()
    alignas(16) T staging;
    T* operator->() { return &staging; }
    size_t NumInstances() const { return m_numInstances; }

    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(size_t instanceIndex = 0)
    {
      return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }
};

// Helper class to create and update a structured buffer.
// Usage: 
//    StructuredBuffer<...> sb;
//    sb.Create(...);
//    sb[index].var = ... ; 
//    sb.CopyStagingToGPU(...);
//    Set...View(..., sb.GputVirtualAddress());
template <class T>
class StructuredBuffer : public GpuUploadBuffer
{
  T* m_mappedBuffers;
  std::vector<T> m_staging;
  size_t m_numInstances;

  public:
    // Performance tip: Align structures on sizeof(float4) boundary.
    // Ref: https://developer.nvidia.com/content/understanding-structured-buffer-performance
    static_assert(sizeof(T) % 16 == 0, "Align structure buffers on 16 byte boundary for performance reasons.");

    StructuredBuffer()
      : m_mappedBuffers(nullptr),
        m_numInstances(0) {}

    virtual ~StructuredBuffer() = default;

    void Create(ID3D12Device5* device, size_t numElements, size_t numInstances = 1, LPCWSTR resourceName = nullptr)
    {
      m_numInstances = numInstances;
      m_staging.resize(numElements);
      size_t bufferSize = numInstances * numElements * sizeof(T);
      Allocate(device, bufferSize, resourceName);
      m_mappedBuffers = reinterpret_cast<T*>(MapCpuWriteOnly());
    }

    void CopyStagingToGpu(size_t instanceIndex = 0)
    {
      memcpy(m_mappedBuffers + instanceIndex * NumElements(), &m_staging[0], InstanceSize());
    }

    auto begin() { return m_staging.begin(); }
    auto end() { return m_staging.end(); }
    auto begin() const { return m_staging.begin(); }
    auto end() const { return m_staging.end(); }

    // Accessors
    T& operator[](size_t elementIndex) { return m_staging[elementIndex]; }
    const T& operator[](size_t elementIndex) const { return m_staging[elementIndex]; }
    [[nodiscard]] size_t NumElements() const { return m_staging.size(); }
    [[nodiscard]] static size_t ElementSize() { return sizeof(T); }
    [[nodiscard]] size_t NumInstances() const { return m_numInstances; }
    [[nodiscard]] size_t InstanceSize() const { return NumElements() * ElementSize(); }

    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0, UINT elementIndex = 0) const
    {
      return m_resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize() + elementIndex * ElementSize();
    }
};
