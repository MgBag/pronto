#include "d3d12_dynamic_descriptor_heap.h"

#include "d3d12_root_signature.h"

#include <assert.h>
#include <stdexcept>

namespace pronto
{
  namespace graphics
  {
    //-----------------------------------------------------------------------------------------------
    DynamicDescriptorHeap::DynamicDescriptorHeap(
      D3D12_DESCRIPTOR_HEAP_TYPE heap_type, ID3D12Device* device, uint32_t num_descriptors_per_heap) :
      descriptor_heap_type_(heap_type),
      num_descriptors_per_heap_(num_descriptors_per_heap),
      descriptor_table_mask_(0),
      stale_descriptor_table_mask_(0),
      current_cpu_descriptor_handle_(D3D12_DEFAULT),
      current_gpu_descriptor_handle_(D3D12_DEFAULT),
      num_free_handles_(0),
      current_descriptor_heap_(0),
      device_(device)
    {
      descriptor_handle_increment_size_ = device_->GetDescriptorHandleIncrementSize(heap_type);
      descriptor_handle_cache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(num_descriptors_per_heap);
    }

    //-----------------------------------------------------------------------------------------------
    DynamicDescriptorHeap::~DynamicDescriptorHeap()
    {
    }

    //-----------------------------------------------------------------------------------------------
    void DynamicDescriptorHeap::StageDescriptors(
      uint32_t root_parameter_index,
      uint32_t offset,
      uint32_t num_descriptors,
      const D3D12_CPU_DESCRIPTOR_HANDLE src_descriptor)
    {
      if (num_descriptors > num_descriptors_per_heap_
        || root_parameter_index >= max_descriptor_tables_)
      {
        throw std::bad_alloc();
      }

      DescriptorTableCache& descriptor_table_cache = descriptor_table_cache_[root_parameter_index];

      if ((offset + num_descriptors) > descriptor_table_cache.num_descriptors_)
      {
        throw std::length_error(
          "Number of descriptors exceeds the number of descriptors in the descriptor table.");
      }

      D3D12_CPU_DESCRIPTOR_HANDLE* dst_descriptor =
        (descriptor_table_cache.base_descriptor_ + offset);
      for (uint32_t i = 0; i < num_descriptors; ++i)
      {
        dst_descriptor[i] =
          CD3DX12_CPU_DESCRIPTOR_HANDLE(src_descriptor, i, descriptor_handle_increment_size_);
      }

      stale_descriptor_table_mask_ |= (1 << root_parameter_index);
    }

    //-----------------------------------------------------------------------------------------------
    void DynamicDescriptorHeap::CommitStagedDescriptors(
      ID3D12GraphicsCommandList* command_list,
      std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> set_func)
    {
      uint32_t num_descriptors_to_commit = ComputeStaleDescriptorCount();

      if (num_descriptors_to_commit > 0)
      {
        assert(command_list != nullptr);

        if (current_descriptor_heap_ == nullptr || num_free_handles_ < num_descriptors_to_commit)
        {

          current_descriptor_heap_ = RequestDescriptorHeap();
          current_cpu_descriptor_handle_ = current_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
          current_gpu_descriptor_handle_ = current_descriptor_heap_->GetGPUDescriptorHandleForHeapStart();
          num_free_handles_ = num_descriptors_per_heap_;

          if (descriptor_heaps_[descriptor_heap_type_] != current_descriptor_heap_)
          {
            descriptor_heaps_[descriptor_heap_type_] = current_descriptor_heap_;

            UINT num_descriptor_heaps = 0;
            ID3D12DescriptorHeap* descriptor_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

            for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
            {
              ID3D12DescriptorHeap* descriptor_heap = descriptor_heaps_[i];
              if (descriptor_heap != nullptr)
              {
                // Jerry, why do you make this copy?
                descriptor_heaps[num_descriptor_heaps++] = descriptor_heap;
              }
            }

            command_list->SetDescriptorHeaps(num_descriptor_heaps, descriptor_heaps);
          }

          // When updating the descriptor heap on the command list, all descriptor
          // tables must be (re)recopied to the new descriptor heap (not just
          // the stale descriptor tables).
          stale_descriptor_table_mask_ = descriptor_table_mask_;
        }

        DWORD root_index;
        // Scan from LSB to MSB for a bit set in staleDescriptorsBitMask
        while (_BitScanForward(&root_index, stale_descriptor_table_mask_))
        {
          UINT num_src_descriptors = descriptor_table_cache_[root_index].num_descriptors_;
          D3D12_CPU_DESCRIPTOR_HANDLE* src_descriptor_handles =
            descriptor_table_cache_[root_index].base_descriptor_;

          D3D12_CPU_DESCRIPTOR_HANDLE dest_descriptor_range_start[] =
          {
              current_cpu_descriptor_handle_
          };
          UINT dest_descriptor_range_sizes[] =
          {
              num_src_descriptors
          };

          // Copy the staged CPU visible descriptors to the GPU visible descriptor heap.
          device_->CopyDescriptors(
            1,
            dest_descriptor_range_start,
            dest_descriptor_range_sizes,
            num_src_descriptors,
            src_descriptor_handles,
            nullptr,
            descriptor_heap_type_);

          // Set the descriptors on the command list using the passed-in setter function.
          set_func(command_list, root_index, current_gpu_descriptor_handle_);

          // Offset current CPU and GPU descriptor handles.
          current_cpu_descriptor_handle_.Offset(
            num_src_descriptors, descriptor_handle_increment_size_);
          current_gpu_descriptor_handle_.Offset(
            num_src_descriptors, descriptor_handle_increment_size_);

          num_free_handles_ -= num_src_descriptors;
          // Flip the stale bit so the descriptor table is not recopied again unless it is updated with a new descriptor.
          stale_descriptor_table_mask_ ^= (1 << root_index);
        }
      }
    }

    //-----------------------------------------------------------------------------------------------
    void DynamicDescriptorHeap::CommitStagedDescriptorsForDraw(ID3D12GraphicsCommandList* command_list)
    {
      CommitStagedDescriptors(
        command_list, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
    }

    //-----------------------------------------------------------------------------------------------
    void DynamicDescriptorHeap::CommitStagedDescriptorsForDispatch(ID3D12GraphicsCommandList* command_list)
    {
      CommitStagedDescriptors(
        command_list, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
    }

    //-----------------------------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CopyDescriptor(
      ID3D12GraphicsCommandList* command_list,
      D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor)
    {
      if (current_descriptor_heap_ == nullptr || num_free_handles_ < 1)
      {
        current_descriptor_heap_ = RequestDescriptorHeap();
        current_cpu_descriptor_handle_ =
          current_descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
        current_gpu_descriptor_handle_ =
          current_descriptor_heap_->GetGPUDescriptorHandleForHeapStart();
        num_free_handles_ = num_descriptors_per_heap_;

        if (descriptor_heaps_[descriptor_heap_type_] != current_descriptor_heap_)
        {
          descriptor_heaps_[descriptor_heap_type_] = current_descriptor_heap_;
          UINT num_descriptor_heaps = 0;
          ID3D12DescriptorHeap* descriptor_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

          for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
          {
            ID3D12DescriptorHeap* descriptor_heap = descriptor_heaps_[i];
            if (descriptor_heap != nullptr)
            {
              // Jerry, why do you make this copy?
              descriptor_heaps[num_descriptor_heaps++] = descriptor_heap;
            }
          }

          command_list->SetDescriptorHeaps(num_descriptor_heaps, descriptor_heaps);
        }

        stale_descriptor_table_mask_ = descriptor_table_mask_;
      }

      D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = current_gpu_descriptor_handle_;

      device_->CopyDescriptorsSimple(
        1, current_cpu_descriptor_handle_, cpu_descriptor, descriptor_heap_type_);

      current_cpu_descriptor_handle_.Offset(1, descriptor_handle_increment_size_);
      current_gpu_descriptor_handle_.Offset(1, descriptor_handle_increment_size_);
      num_free_handles_ -= 1;

      return gpu_handle;
    }

    //-----------------------------------------------------------------------------------------------
    void DynamicDescriptorHeap::ParseRootSignature(D3D12_ROOT_SIGNATURE_DESC1 desc, uint32_t bit_mask, uint32_t num_desc)
    {
      stale_descriptor_table_mask_ = 0;

      descriptor_table_mask_ = bit_mask; // becuase its the first, right? pls help
      uint32_t descriptor_table_mask = descriptor_table_mask_;

      uint32_t current_offset = 0;
      DWORD root_index;

      while (
        _BitScanForward(&root_index, descriptor_table_mask) && root_index < desc.NumParameters)
      {
        uint32_t num_descriptors = num_desc;

        DescriptorTableCache& descriptor_table_cache = descriptor_table_cache_[root_index];
        descriptor_table_cache.num_descriptors_ = num_descriptors;
        descriptor_table_cache.base_descriptor_ = descriptor_handle_cache.get() + current_offset;

        current_offset += num_descriptors;

        // Flip the descriptor table bit so it's not scanned again for the current index.
        descriptor_table_mask ^= (1 << root_index);
      }

      assert(current_offset <= num_descriptors_per_heap_
        && "The root signature requires more than the maximum number of descriptors per descriptor"
        && "heap. Consider increasing the maximum number of descriptors per descriptor heap.");
    }

    //-----------------------------------------------------------------------------------------------
    void DynamicDescriptorHeap::Reset()
    {
      available_descriptor_heaps_ = descriptor_heap_pool_;
      current_descriptor_heap_ = nullptr;
      current_cpu_descriptor_handle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
      current_gpu_descriptor_handle_ = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
      num_free_handles_ = 0;
      descriptor_table_mask_ = 0;
      stale_descriptor_table_mask_ = 0;

      // Reset the table cache
      for (int i = 0; i < max_descriptor_tables_; ++i)
      {
        descriptor_table_cache_[i].Reset();
      }

      for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
      {
        descriptor_heaps_[i] = nullptr;
      }
    }

    //-----------------------------------------------------------------------------------------------
    ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap()
    {
      ID3D12DescriptorHeap* descriptor_heap;

      if (available_descriptor_heaps_.empty() == false)
      {
        descriptor_heap = available_descriptor_heaps_.front();
        available_descriptor_heaps_.pop();
      }
      else
      {
        descriptor_heap = CreateDescriptorHeap();
        descriptor_heap_pool_.push(descriptor_heap);
      }

      return descriptor_heap;
    }

    //-----------------------------------------------------------------------------------------------
    ID3D12DescriptorHeap* DynamicDescriptorHeap::CreateDescriptorHeap()
    {
      D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
      descriptor_heap_desc.Type = descriptor_heap_type_;
      descriptor_heap_desc.NumDescriptors = num_descriptors_per_heap_;
      descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

      ID3D12DescriptorHeap* descriptor_heap;
      assert(SUCCEEDED(
        device_->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&descriptor_heap))));

      descriptor_heap->SetName(L"created dynamic descriptor heap");

      return descriptor_heap;
    }

    //-----------------------------------------------------------------------------------------------
    uint32_t DynamicDescriptorHeap::ComputeStaleDescriptorCount() const
    {
      uint32_t num_stale_descriptors = 0;
      DWORD i;
      DWORD stale_descriptors_mask = stale_descriptor_table_mask_;

      while (_BitScanForward(&i, stale_descriptors_mask))
      {
        num_stale_descriptors += descriptor_table_cache_[i].num_descriptors_;
        stale_descriptors_mask ^= (1 << i);
      }

      return num_stale_descriptors;
    }
  }
}