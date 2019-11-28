#pragma once

#include <cstdint>

struct ID3D12Resource;
struct ID3D12Device2;
struct ID3D12GraphicsCommandList;

typedef uint64_t D3D12_GPU_VIRTUAL_ADDRESS;

namespace pronto
{
  namespace graphics
  {
    /**
    * @author Yana Mateeva, Benjamin Waanders
    */
    class D3D12ConstantBufferHeap
    {
    public:
      void Create(ID3D12Device2* device);

      ~D3D12ConstantBufferHeap();

      void OnDestroy();

      D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();

     void Write(
       const void* data, 
       size_t size,
       ID3D12GraphicsCommandList* command_list, 
       uint32_t root_parameter, 
       bool is_compute = false);

     D3D12_GPU_VIRTUAL_ADDRESS Upload(
       const void* data,
       size_t size
     );

    private:
      ID3D12Resource* heap_ = nullptr;
      static const size_t heap_size_ = 8194304; 
      size_t ptr_ = 0;

      uint8_t* data_begin_;
    };
  }
}