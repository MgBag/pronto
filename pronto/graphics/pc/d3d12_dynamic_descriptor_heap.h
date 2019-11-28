#pragma once

#include "d3dx12.h"

#include <wrl.h>

#include <cstdint>
#include <memory>
#include <queue>
#include <functional>

namespace pronto
{
  namespace graphics
  {
    class RootSignature;

    class DynamicDescriptorHeap
    {
    public:
      DynamicDescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE heap_type,
        ID3D12Device* device,
        uint32_t num_descriptors_per_heap = 1024);

      virtual ~DynamicDescriptorHeap();

      void StageDescriptors(
        uint32_t root_parameter_index,
        uint32_t offset,
        uint32_t num_descriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE src_descriptor);

      void CommitStagedDescriptors(
        ID3D12GraphicsCommandList* command_list,
        std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> set_func);

      void CommitStagedDescriptorsForDraw(ID3D12GraphicsCommandList* command_list);
      void CommitStagedDescriptorsForDispatch(ID3D12GraphicsCommandList* command_list);

      D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(
        ID3D12GraphicsCommandList* comand_list, D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor);

      void ParseRootSignature(D3D12_ROOT_SIGNATURE_DESC1 desc, uint32_t bit_mask, uint32_t num_desc);

      void Reset();
    private:

      static const uint32_t max_descriptor_tables_ = 32;

      struct DescriptorTableCache
      {
        DescriptorTableCache()
          : num_descriptors_(0)
          , base_descriptor_(nullptr)
        {
        }

        void Reset()
        {
          num_descriptors_ = 0;
          base_descriptor_ = nullptr;
        }

        uint32_t num_descriptors_;
        D3D12_CPU_DESCRIPTOR_HANDLE* base_descriptor_;
      };

      D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type_;

      uint32_t num_descriptors_per_heap_;

      uint32_t descriptor_handle_increment_size_;

      std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> descriptor_handle_cache;

      DescriptorTableCache descriptor_table_cache_[max_descriptor_tables_];

      uint32_t descriptor_table_mask_;
      uint32_t stale_descriptor_table_mask_;

      using DescriptorHeapPool = std::queue<ID3D12DescriptorHeap*>;

      DescriptorHeapPool descriptor_heap_pool_;
      DescriptorHeapPool available_descriptor_heaps_;

      ID3D12DescriptorHeap* current_descriptor_heap_;
      CD3DX12_GPU_DESCRIPTOR_HANDLE current_gpu_descriptor_handle_;
      CD3DX12_CPU_DESCRIPTOR_HANDLE current_cpu_descriptor_handle_;
      ID3D12Device* device_;

      ID3D12DescriptorHeap* descriptor_heaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

      uint32_t num_free_handles_;

      ID3D12DescriptorHeap* RequestDescriptorHeap();
      ID3D12DescriptorHeap* CreateDescriptorHeap();

      uint32_t ComputeStaleDescriptorCount() const;
    };
  }
}