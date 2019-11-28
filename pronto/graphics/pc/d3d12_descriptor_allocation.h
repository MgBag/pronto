#pragma once
#include <d3d12.h>

#include <cstdint>
#include <memory>

namespace pronto
{
  namespace graphics
  {
    class DescriptorAllocatorPage;

    class DescriptorAllocation
    {
    public:
      DescriptorAllocation();

      DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t num_handles, uint32_t descriptor_size, std::shared_ptr<DescriptorAllocatorPage> page);

      ~DescriptorAllocation();

      DescriptorAllocation(const DescriptorAllocation&) = delete;
      DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

      DescriptorAllocation(DescriptorAllocation&& allocation);
      DescriptorAllocation& operator=(DescriptorAllocation&& other);

      bool IsNull() const;

      D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;

      uint32_t num_handles() const;

      std::shared_ptr<DescriptorAllocatorPage> page() const;

    private:
      void Free();

      D3D12_CPU_DESCRIPTOR_HANDLE descriptor_;
      uint32_t num_handles_;
      uint32_t descriptor_size_;

      std::shared_ptr<DescriptorAllocatorPage> page_;
    };
  }
}
