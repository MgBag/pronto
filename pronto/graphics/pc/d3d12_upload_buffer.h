#pragma once
#include <d3d12.h>
#include <memory>
#include <deque>

namespace pronto
{
  namespace graphics
  {
    class UploadBuffer
    {
    public:
      struct Allocation
      {
        void* CPU;
        D3D12_GPU_VIRTUAL_ADDRESS GPU;
      };

      explicit UploadBuffer(ID3D12Device* device, size_t page_size = 1024 * 1024);

      virtual ~UploadBuffer();

      size_t page_size() const;

      Allocation Allocate(size_t size_in_bytes, size_t alignment);

      void Reset();

    private:
      struct Page
      {
        Page(ID3D12Device* device, size_t size_in_bytes);
        ~Page();

        bool HasSpace(size_t size_in_bytes, size_t alignment) const;

        Allocation Allocate(size_t size_in_bytes, size_t alignment);

        void Reset();

      private:
        ID3D12Resource* resource_;

        void* cpu_;
        D3D12_GPU_VIRTUAL_ADDRESS gpu_;

        size_t page_size_;
        size_t offset_;
        ID3D12Device* device_;
      };

      std::shared_ptr<Page> RequestPage();

      using Pool = std::deque<std::shared_ptr<Page>>;
      Pool page_pool_;
      Pool unused_pages_;

      std::shared_ptr<Page> current_page_;

      size_t page_size_;

      ID3D12Device* device_;
    };
  }
}