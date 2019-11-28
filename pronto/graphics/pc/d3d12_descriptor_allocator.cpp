#include "d3d12_descriptor_allocator.h"
#include "d3d12_descriptor_allocator_page.h"


namespace pronto
{
  namespace graphics
  {
    //-----------------------------------------------------------------------------------------------
    DescriptorAllocator::DescriptorAllocator(
      D3D12_DESCRIPTOR_HEAP_TYPE type,
      ID3D12Device* device,
      uint32_t descriptors_per_heap) :
      heap_type_(type),
      descriptors_per_heap_(descriptors_per_heap),
      device_(device)
    {
    }

    //-----------------------------------------------------------------------------------------------
    DescriptorAllocator::~DescriptorAllocator()
    {
    }

    //-----------------------------------------------------------------------------------------------
    DescriptorAllocation DescriptorAllocator::Allocate(uint32_t descriptors)
    {
      //std::lock_guard<std::mutex> lock(allocation_mutex_);

      DescriptorAllocation allocation;

      for (auto iter = available_heaps_.begin(); iter != available_heaps_.end(); ++iter)
      {
        auto allocator_page = heap_pool_[*iter];

        allocation = allocator_page->Allocate(descriptors);

        if (allocator_page->NumFreeHandles() == 0)
        {
          iter = available_heaps_.erase(iter);
        }

        if (allocation.IsNull() == false)
        {
          break;
        }
      }

      if (allocation.IsNull())
      {
        descriptors_per_heap_ = std::max(descriptors_per_heap_, descriptors);
        auto page = CreateAllocatorPage();

        allocation = page->Allocate(descriptors);
      }

      return allocation;
    }

    //-----------------------------------------------------------------------------------------------
    void DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frame_number)
    {
      //std::lock_guard<std::mutex> lock(allocation_mutex_);

      for (size_t i = 0; i < heap_pool_.size(); ++i)
      {
        auto page = heap_pool_[i];

        page->ReleaseStaleDescriptors(frame_number);

        if (page->NumFreeHandles() > 0)
        {
          available_heaps_.insert(i);
        }
      }
    }

    //-----------------------------------------------------------------------------------------------
    std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
    {
      auto new_page = std::make_shared<DescriptorAllocatorPage>(heap_type_, descriptors_per_heap_, device_);

      heap_pool_.emplace_back(new_page);
      available_heaps_.insert(heap_pool_.size() - 1);

      return new_page;
    }
  }
}