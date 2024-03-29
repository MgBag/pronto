#pragma once
#include "d3d12_descriptor_allocation.h"

#include "d3dx12.h"

#include <cstdint>
#include <mutex>
#include <memory>
#include <set>
#include <vector>

namespace pronto
{
  namespace graphics
  {
    class DescriptorAllocatorPage;

    class DescriptorAllocator
    {
    public:
      DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12Device* device, uint32_t descriptors_per_heap = 256);
      virtual ~DescriptorAllocator();

      /**
       * Allocate a number of contiguous descriptors from a CPU visible descriptor heap.
       *
       * @param numDescriptors The number of contiguous descriptors to allocate.
       * Cannot be more than the number of descriptors per descriptor heap.
       */
      DescriptorAllocation Allocate(uint32_t descriptors = 1);

      /**
       * When the frame has completed, the stale descriptors can be released.
       */
      void ReleaseStaleDescriptors(uint64_t frame_number);

    private:
      using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

      // Create a new heap with a specific number of descriptors.
      std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();

      D3D12_DESCRIPTOR_HEAP_TYPE heap_type_;
      uint32_t descriptors_per_heap_;

      DescriptorHeapPool heap_pool_;
      // Indices of available heaps in the heap pool.
      std::set<size_t> available_heaps_;

      std::mutex allocation_mutex_;

      ID3D12Device* device_;
    };
  }
}