#include "d3d12_upload_buffer.h"
#include "platform/utils.h"
#include <assert.h>

namespace pronto
{
  namespace graphics
  {
    UploadBuffer::UploadBuffer(ID3D12Device* device, size_t page_size) :
      page_size_(page_size),
      device_(device)
    {
    }

    UploadBuffer::~UploadBuffer()
    {
    }

    size_t UploadBuffer::page_size() const
    {
      return page_size_;
    }

    UploadBuffer::Allocation UploadBuffer::Allocate(size_t size, size_t alignment)
    {
      assert(size < page_size_ && "Page size not big enough");

      if (current_page_ == nullptr
        || current_page_->HasSpace(size, alignment) == false)
      {
        current_page_ = RequestPage();
      }

      return current_page_->Allocate(size, alignment);
    }

    void UploadBuffer::Reset()
    {
      unused_pages_ = page_pool_;
      current_page_ = nullptr;

      for (auto page : unused_pages_)
      {
        page->Reset();
      }
    }

    std::shared_ptr<UploadBuffer::Page> UploadBuffer::RequestPage()
    {
      std::shared_ptr<Page> page;

      if (unused_pages_.size() > 0)
      {
        page = unused_pages_.front();
        unused_pages_.pop_front();
      }
      else
      {
        page = std::make_shared<Page>(device_, page_size_);
        page_pool_.push_back(page);
      }

      return page;
    }

    UploadBuffer::Page::Page(ID3D12Device* device, size_t size) :
      page_size_(size),
      offset_(0),
      cpu_(nullptr),
      gpu_(D3D12_GPU_VIRTUAL_ADDRESS(0)),
      device_(device)
    {
      assert(SUCCEEDED(
        device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(page_size_),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource_))));

      gpu_ = resource_->GetGPUVirtualAddress();
      resource_->Map(0, nullptr, &cpu_);
      resource_->SetName(L"Upload buffer page thingy");
    }

    UploadBuffer::Page::~Page()
    {
      resource_->Unmap(0, nullptr);
      cpu_ = nullptr;
      gpu_ = D3D12_GPU_VIRTUAL_ADDRESS(0);
    }

    bool UploadBuffer::Page::HasSpace(size_t size, size_t alignment) const
    {
      size_t alignedSize = math::AlignUp(size, alignment);
      size_t alignedOffset = math::AlignUp(offset_, alignment);

      return alignedOffset + alignedSize <= page_size_;
    }

    UploadBuffer::Allocation UploadBuffer::Page::Allocate(size_t size, size_t alignment)
    {
      if (HasSpace(size, alignment) == false)
      {
        assert(size > page_size_ && "Page not big enough");
      }

      size_t aligned_size = math::AlignUp(size, alignment);
      offset_ = math::AlignUp(offset_, alignment);

      Allocation allocation;
      allocation.CPU = static_cast<uint8_t*>(cpu_) + offset_;
      allocation.GPU = gpu_ + offset_;

      offset_ += aligned_size;

      return allocation;
    }

    void UploadBuffer::Page::Reset()
    {
      offset_ = 0;
    }
  }
}