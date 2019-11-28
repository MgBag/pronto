#include "d3d12_constant_buffer_heap.h"
#include <assert.h>

#include "d3dx12.h"
#include <d3d12.h>


namespace pronto
{
  namespace graphics
  {
    //------------------------------------------------------------------------------------------------------
    void D3D12ConstantBufferHeap::Create(ID3D12Device2* device)
    {
      D3D12_HEAP_PROPERTIES properties = {};
      properties.CreationNodeMask = 0;
      properties.VisibleNodeMask = 0;
      properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      properties.Type = D3D12_HEAP_TYPE_UPLOAD;

      device->CreateCommittedResource(
        &properties,
        D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
        &CD3DX12_RESOURCE_DESC::Buffer(heap_size_),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&heap_)
      );

      heap_->SetName(L"yana pls");

      CD3DX12_RANGE range(0, 0);
     
      assert(SUCCEEDED(
        heap_->Map(0, &range, reinterpret_cast<void**>(&data_begin_))));
    }

    //---------------------------------------------------------------------------------------------
    D3D12ConstantBufferHeap::~D3D12ConstantBufferHeap()
    {
    }

    //------------------------------------------------------------------------------------------------------
    void D3D12ConstantBufferHeap::OnDestroy()
    {
      if (heap_ != nullptr)
      {
        heap_->Unmap(0, nullptr);

        heap_->Release();
      }

      heap_ = nullptr;
    }

    //------------------------------------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS D3D12ConstantBufferHeap::GetGPUVirtualAddress()
    {
      return heap_->GetGPUVirtualAddress();
    }

    //------------------------------------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS D3D12ConstantBufferHeap::Upload(
      const void* data,
      size_t size)
    {
      //If the data will overflow
      if (ptr_ + size > heap_size_)
      {
        //Return to the beginning of the heap
        ptr_ = 0;
      }

      memcpy(data_begin_ + ptr_, data, size);
      D3D12_GPU_VIRTUAL_ADDRESS data_gpu_address = heap_->GetGPUVirtualAddress() + ptr_;

      //Advance pointer
      ptr_ += size;
      if (size % 256 != 0)
      {
        ptr_ += 256 - (size % 256);
      }

      return data_gpu_address;
    }

    //------------------------------------------------------------------------------------------------------
    void D3D12ConstantBufferHeap::Write(
      const void* data, 
      size_t size, 
      ID3D12GraphicsCommandList* command_list, 
      UINT root_parameter,
      bool is_compute)
    {
      //If the data will overflow
      if (ptr_ + size > heap_size_)
      {
        //Return to the beginning of the heap
        ptr_ = 0;
        assert(false && "rip buffer");
      }

      memcpy(data_begin_ + ptr_, data, size);

      if (is_compute)
      {
        command_list->SetComputeRootConstantBufferView(
          root_parameter, heap_->GetGPUVirtualAddress() + ptr_);
      }
      else
      {
        command_list->SetGraphicsRootConstantBufferView(
          root_parameter, heap_->GetGPUVirtualAddress() + ptr_);
      }

      ptr_ += size;
      if (size % 256 != 0)
      {
        ptr_ += 256 - (size % 256);
      }
    }
  }
}