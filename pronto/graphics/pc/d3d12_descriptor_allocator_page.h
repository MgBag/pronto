#pragma once

#include "d3d12_descriptor_allocator.h"

#include <d3d12.h>

#include <wrl.h>

#include <map>
#include <memory>
#include <mutex>
#include <queue>

namespace pronto
{
  namespace graphics
  {
    class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
    {
    public:
      DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors, ID3D12Device* device);

      D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;

      bool HasSpace(uint32_t num_descriptors) const;

      uint32_t NumFreeHandles() const;

      DescriptorAllocation Allocate(uint32_t num_descriptors);

      void Free(DescriptorAllocation&& descriptor_dandle, uint64_t frame_number);

      void ReleaseStaleDescriptors(uint64_t frame_number);

    protected:

      uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);

      void AddNewBlock(uint32_t offset, uint32_t num_descriptors);

      void FreeBlock(uint32_t offset, uint32_t num_descriptors);

    private:
      using OffsetType = uint32_t;
      using SizeType = uint32_t;

      struct FreeBlockInfo;
      using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;

      using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

      struct FreeBlockInfo
      {
        FreeBlockInfo(SizeType size)
          : size_(size)
        {}

        SizeType size_;
        FreeListBySize::iterator free_list_by_size_it_;
      };

      struct StaleDescriptorInfo
      {
        StaleDescriptorInfo(OffsetType offset, SizeType size, uint64_t frame)
          : offset_(offset)
          , size_(size)
          , frame_number_(frame)
        {}

        OffsetType offset_;
        SizeType size_;
        uint64_t frame_number_;
      };

      using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

      FreeListByOffset free_list_by_offset_;
      FreeListBySize free_list_by_size_;
      StaleDescriptorQueue stale_descriptors_;

      ID3D12Device* device_;

      ID3D12DescriptorHeap* descriptor_heap_;
      D3D12_DESCRIPTOR_HEAP_TYPE heap_type_;
      CD3DX12_CPU_DESCRIPTOR_HANDLE base_descriptor_;
      uint32_t descriptor_handle_increment_size_;
      uint32_t num_descriptors_in_heap_;
      uint32_t num_free_handles_;

      std::mutex allocation_mutex_;
    };
  }
}