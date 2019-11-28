#include "d3d12_descriptor_allocator_page.h"
#include <assert.h>

namespace pronto
{
  namespace graphics
  {
    //-----------------------------------------------------------------------------------------------
    DescriptorAllocatorPage::DescriptorAllocatorPage(
      D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t num_descriptors, ID3D12Device* device) :
      device_(device),
      descriptor_handle_increment_size_(0),
      heap_type_(type),
      num_descriptors_in_heap_(num_descriptors),
      num_free_handles_(0)
    {
      D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
      heap_desc.Type = heap_type_;
      heap_desc.NumDescriptors = num_descriptors;

      assert(SUCCEEDED(
        device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptor_heap_))));
      descriptor_heap_->SetName(L"Descriptor allocator page descriptor heap");
      base_descriptor_ = descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
      descriptor_handle_increment_size_ = device->GetDescriptorHandleIncrementSize(heap_type_);
      num_free_handles_ = num_descriptors;

      // Initialize the free lists
      AddNewBlock(0, num_free_handles_);
    }

    //-----------------------------------------------------------------------------------------------
    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::GetHeapType() const
    {
      return heap_type_;
    }

    //-----------------------------------------------------------------------------------------------
    bool DescriptorAllocatorPage::HasSpace(uint32_t num_descriptors) const
    {
      return free_list_by_size_.lower_bound(num_descriptors) != free_list_by_size_.end();
    }

    //-----------------------------------------------------------------------------------------------
    uint32_t DescriptorAllocatorPage::NumFreeHandles() const
    {
      return num_free_handles_;
    }

    //-----------------------------------------------------------------------------------------------
    DescriptorAllocation DescriptorAllocatorPage::Allocate(uint32_t num_descriptors)
    {
      //std::lock_guard<std::mutex> lock(allocation_mutex_);

      if (num_descriptors > num_free_handles_)
      {
        return DescriptorAllocation();
      }

      auto smallest_block = free_list_by_size_.lower_bound(num_descriptors);
      if (smallest_block == free_list_by_size_.end())
      {
        return DescriptorAllocation();
      }

      auto block_size = smallest_block->first;
      auto offset_it = smallest_block->second;
      auto offset = offset_it->first;

      free_list_by_size_.erase(smallest_block);
      free_list_by_offset_.erase(offset_it);

      // Compute the new free block that results from splitting this block.
      auto new_offset = offset + num_descriptors;
      auto new_size = block_size - num_descriptors;

      if (new_size > 0)
      {
        AddNewBlock(new_offset, new_size);
      }

      // Decrement free handles.
      num_free_handles_ -= num_descriptors;

      return DescriptorAllocation(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(base_descriptor_, offset, descriptor_handle_increment_size_),
        num_descriptors,
        descriptor_handle_increment_size_,
        shared_from_this());
    }

    //-----------------------------------------------------------------------------------------------
    void DescriptorAllocatorPage::Free(
      DescriptorAllocation&& descriptor_handle,
      uint64_t frame_number)
    {
      auto offset = ComputeOffset(descriptor_handle.GetDescriptorHandle());

      //std::lock_guard<std::mutex> lock(allocation_mutex_);

      // Don't add the block directly to the free list until the frame has completed.
      stale_descriptors_.emplace(offset, descriptor_handle.num_handles(), frame_number);
    }

    //-----------------------------------------------------------------------------------------------
    void DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frame_number)
    {
      //std::lock_guard<std::mutex> lock(allocation_mutex_);

      while (!stale_descriptors_.empty() && stale_descriptors_.front().frame_number_ <= frame_number)
      {
        auto& stale_desc = stale_descriptors_.front();

        // The offset of the descriptor in the heap.
        auto offset = stale_desc.offset_;
        // The number of descriptors that were allocated.
        auto num_descriptors = stale_desc.size_;

        FreeBlock(offset, num_descriptors);

        stale_descriptors_.pop();
      }
    }

    //-----------------------------------------------------------------------------------------------
    uint32_t DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
      return
        static_cast<uint32_t>(handle.ptr - base_descriptor_.ptr) / descriptor_handle_increment_size_;
    }

    //-----------------------------------------------------------------------------------------------
    void DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t num_descriptors)
    {
      auto offset_it = free_list_by_offset_.emplace(offset, num_descriptors);
      auto size_it = free_list_by_size_.emplace(num_descriptors, offset_it.first);

      offset_it.first->second.free_list_by_size_it_ = size_it;
    }

    //-----------------------------------------------------------------------------------------------
    void DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t num_descriptors)
    {
      auto next_block = free_list_by_offset_.upper_bound(offset);

      auto prev_block = next_block;
      if (next_block != free_list_by_offset_.begin())
      {
        --prev_block;
      }
      else
      {
        prev_block = free_list_by_offset_.end();
      }

      num_free_handles_ += num_descriptors;

      if (prev_block != free_list_by_offset_.end() &&
        offset == prev_block->first + prev_block->second.size_)
      {
        offset = prev_block->first;
        num_descriptors += prev_block->second.size_;

        // Remove the previous block from the free list.
        free_list_by_size_.erase(prev_block->second.free_list_by_size_it_);
        free_list_by_offset_.erase(prev_block);
      }

      if (next_block != free_list_by_offset_.end() &&
        offset + num_descriptors == next_block->first)
      {
        num_descriptors += next_block->second.size_;

        // Remove the next block from the free list.
        free_list_by_size_.erase(next_block->second.free_list_by_size_it_);
        free_list_by_offset_.erase(next_block);
      }

      // Add the freed block to the free list.
      AddNewBlock(offset, num_descriptors);
    }
  }
}