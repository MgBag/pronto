#pragma once
#include <d3d12.h>
#include <stdint.h>

namespace pronto
{
  namespace graphics
  {
    class DynamicDescriptorHeap;
    class UploadBuffer;
    class D3D12ConstantBufferHeap;
    class CommandList
    {
    public:
      CommandList(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type);

      ID3D12GraphicsCommandList* command_list();
      DynamicDescriptorHeap* descriptor_heap();
      UploadBuffer* upload_buffer();
      D3D12ConstantBufferHeap* const_buffer_heap();

      ID3D12Device2* device();
      void SetGraphicsRootSignature(
        ID3D12RootSignature* root_sig,
        D3D12_ROOT_SIGNATURE_DESC1 desc,
        uint32_t bit_mask,
        uint32_t num_desc);

      void Reset();

    private:
      ID3D12CommandAllocator* allocator_;
      ID3D12GraphicsCommandList* command_list_;
      D3D12_COMMAND_LIST_TYPE type_;
      ID3D12Device2* device_;
      DynamicDescriptorHeap* dynamic_descriptor_heap_;
      UploadBuffer* upload_buffer_;
      D3D12ConstantBufferHeap* const_buffer_heap_;
    };
  }
}
