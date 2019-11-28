#include "d3d12_command_list.h"
#include "d3d12_dynamic_descriptor_heap.h"
#include "d3d12_upload_buffer.h"
#include "d3d12_constant_buffer_heap.h"

#include <assert.h>

namespace pronto
{
  namespace graphics
  {
    //---------------------------------------------------------------------------------------------
    CommandList::CommandList(
      ID3D12Device2* device,
      D3D12_COMMAND_LIST_TYPE type) :
      type_(type),
      device_(device)
    {
      assert(SUCCEEDED(
        device_->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator_))));
      
      assert(SUCCEEDED(
        device_->CreateCommandList(
          0,
          type_,
          allocator_,
          nullptr,
          IID_PPV_ARGS(&command_list_))));

      dynamic_descriptor_heap_ = new DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, device_);

      upload_buffer_ = new UploadBuffer(device_);
      const_buffer_heap_ = new D3D12ConstantBufferHeap();
      const_buffer_heap_->Create(device_);
    }

    void CommandList::SetGraphicsRootSignature(
      ID3D12RootSignature* root_sig,
      D3D12_ROOT_SIGNATURE_DESC1 desc, 
      uint32_t bit_mask, 
      uint32_t num_desc)
    {
        dynamic_descriptor_heap_->ParseRootSignature(desc, bit_mask, num_desc);

        command_list_->SetGraphicsRootSignature(root_sig);
    }

    ID3D12GraphicsCommandList* CommandList::command_list()
    {
      return command_list_;
    }

    DynamicDescriptorHeap* CommandList::descriptor_heap()
    {
      return dynamic_descriptor_heap_;
    }

    UploadBuffer* CommandList::upload_buffer()
    {
      return upload_buffer_;
    }

    D3D12ConstantBufferHeap* CommandList::const_buffer_heap()
    {
      return const_buffer_heap_;
    }

    ID3D12Device2* CommandList::device()
    {
      return device_;
    }

    void CommandList::Reset()
    {
      assert(SUCCEEDED(allocator_->Reset()));
      assert(SUCCEEDED(command_list_->Reset(allocator_, nullptr)));

      upload_buffer_->Reset();

      dynamic_descriptor_heap_->Reset();
    }
  }
}
