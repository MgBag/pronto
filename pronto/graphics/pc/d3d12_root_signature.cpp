#include "d3d12_root_signature.h"
#include "d3dx12.h"
#include <assert.h>

namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  RootSignature::RootSignature() :
    root_sig_desc_{ 0 },
    root_sig_(0),
    sampler_table_mask_(0),
    descriptor_table_mask_(0),
    device_(nullptr)
  {
  }

  //-----------------------------------------------------------------------------------------------
  RootSignature::RootSignature(
    const D3D12_ROOT_SIGNATURE_DESC1& root_signature_desc, 
    D3D_ROOT_SIGNATURE_VERSION root_signature_version,
    ID3D12Device* device) :
    root_sig_desc_{ 0 },
    root_sig_(0),
    sampler_table_mask_(0),
    descriptor_table_mask_(0),
    device_(device)
  {
    SetRootSignatureDesc(root_signature_desc, root_signature_version);
  }

  //-----------------------------------------------------------------------------------------------
  RootSignature::~RootSignature()
  {
    Destroy();
  }

  //-----------------------------------------------------------------------------------------------
  void RootSignature::Destroy()
  {
    for (UINT i = 0; i < root_sig_desc_.NumParameters; ++i)
    {
      const D3D12_ROOT_PARAMETER1& root_param = root_sig_desc_.pParameters[i];
      if (root_param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
      {
        delete[] root_param.DescriptorTable.pDescriptorRanges;
      }
    }

    delete[] root_sig_desc_.pParameters;
    root_sig_desc_.pParameters = nullptr;
    root_sig_desc_.NumParameters = 0;

    delete[] root_sig_desc_.pStaticSamplers;
    root_sig_desc_.pStaticSamplers = nullptr;
    root_sig_desc_.NumStaticSamplers = 0;

    descriptor_table_mask_ = 0;
    sampler_table_mask_ = 0;

    memset(num_descriptors_per_table_, 0, sizeof(num_descriptors_per_table_));
  }
  //-----------------------------------------------------------------------------------------------
  ID3D12RootSignature* RootSignature::root_signature() const
  {
    return root_sig_;
  }

  //-----------------------------------------------------------------------------------------------
  void RootSignature::SetRootSignatureDesc(
    const D3D12_ROOT_SIGNATURE_DESC1& root_sig_desc,
    D3D_ROOT_SIGNATURE_VERSION root_sig_version)
  {
    // Make sure any previously allocated root signature description is cleaned 
   // up first.
    Destroy();

    UINT num_parameters = root_sig_desc.NumParameters;
    D3D12_ROOT_PARAMETER1* parameters = 
      num_parameters > 0 ? new D3D12_ROOT_PARAMETER1[num_parameters] : nullptr;

    for (UINT i = 0; i < num_parameters; ++i)
    {
      const D3D12_ROOT_PARAMETER1& root_parameter = root_sig_desc.pParameters[i];
      parameters[i] = root_parameter;

      if (root_parameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
      {
        UINT num_descriptor_ranges = root_parameter.DescriptorTable.NumDescriptorRanges;
        D3D12_DESCRIPTOR_RANGE1* descriptor_ranges = 
          num_parameters > 0 ? new D3D12_DESCRIPTOR_RANGE1[num_descriptor_ranges] : nullptr;

        memcpy(descriptor_ranges, root_parameter.DescriptorTable.pDescriptorRanges,
          sizeof(D3D12_DESCRIPTOR_RANGE1) * num_descriptor_ranges);

        parameters[i].DescriptorTable.NumDescriptorRanges = num_descriptor_ranges;
        parameters[i].DescriptorTable.pDescriptorRanges = descriptor_ranges;

        if (num_descriptor_ranges > 0)
        {
          switch (descriptor_ranges[0].RangeType)
          {
          case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
          case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
          case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            descriptor_table_mask_ |= (1 << i);
            break;
          case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
            sampler_table_mask_ |= (1 << i);
            break;
          }
        }

        // Count the number of descriptors in the descriptor table.
        for (UINT j = 0; j < num_descriptor_ranges; ++j)
        {
          num_descriptors_per_table_[i] += descriptor_ranges[j].NumDescriptors;
        }
      }
    }

    root_sig_desc_.NumParameters = num_parameters;
    root_sig_desc_.pParameters = parameters;

    UINT num_static_samplers = root_sig_desc.NumStaticSamplers;
    D3D12_STATIC_SAMPLER_DESC* static_samplers = 
      num_static_samplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[num_static_samplers] : nullptr;

    if (static_samplers)
    {
      memcpy(static_samplers, root_sig_desc.pStaticSamplers,
        sizeof(D3D12_STATIC_SAMPLER_DESC) * num_static_samplers);
    }

    root_sig_desc_.NumStaticSamplers = num_static_samplers;
    root_sig_desc_.pStaticSamplers = static_samplers;

    D3D12_ROOT_SIGNATURE_FLAGS flags = root_sig_desc.Flags;
    root_sig_desc_.Flags = flags;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC version_root_sig_desc;
    version_root_sig_desc.Init_1_1(
      num_parameters, parameters, num_static_samplers, static_samplers, flags);

    // Serialize the root signature.
    ID3DBlob* root_sig_blob;
    ID3DBlob* error_blob;

    HRESULT hr = D3DX12SerializeVersionedRootSignature(
      &version_root_sig_desc,
      root_sig_version,
      &root_sig_blob,
      &error_blob);

    if (error_blob != nullptr || FAILED(hr))
    {
      OutputDebugStringA((char*)error_blob->GetBufferPointer());
      DWORD error = ::GetLastError();
      assert(false);
    }

    assert(SUCCEEDED(device_->CreateRootSignature(
      0, 
      root_sig_blob->GetBufferPointer(),
      root_sig_blob->GetBufferSize(),
      IID_PPV_ARGS(&root_sig_))));

    root_sig_->SetName(L"Main root sig");
  }

  //-----------------------------------------------------------------------------------------------
  const D3D12_ROOT_SIGNATURE_DESC1& RootSignature::root_signature_desc() const
  {
    return root_sig_desc_;
  }

  //-----------------------------------------------------------------------------------------------
  uint32_t RootSignature::GetDescriptorTableBitMask(
    D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type) const
  {
    uint32_t descriptor_table_mask = 0;

    switch (descriptor_heap_type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
      descriptor_table_mask = descriptor_table_mask_;
      break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
      descriptor_table_mask = sampler_table_mask_;
      break;
    }

    return descriptor_table_mask;
  }

  //-----------------------------------------------------------------------------------------------
  uint32_t RootSignature::GetNumDescriptors(uint32_t root_index) const
  {
    assert(root_index < 32);
    return num_descriptors_per_table_[root_index];
  }
}
