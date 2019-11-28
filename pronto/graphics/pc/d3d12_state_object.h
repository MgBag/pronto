#pragma once
#include "d3dx12.h"
#include "resource/pc/model_resource.h" 
#include <vector>

namespace pronto
{
  namespace graphics
  {
    // what about an array for everything that needs keeping
    
    class StateObject
    {
    public:
      StateObject() { };
      StateObject(ID3D12Device2* device);

      ID3D12PipelineState* PSO(PrimitiveType type);
      ID3D12RootSignature* RootSig(PrimitiveType type);
      CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSigDesc(PrimitiveType type);

    private:
      ID3D12Device2* device_;

      std::vector<bool> is_init_;
      std::vector<ID3D12PipelineState*> pso_;
      std::vector<ID3D12RootSignature*> root_signature_;
      std::vector<CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC> root_signature_description_;

      void InitColor();
      void InitTexture();
      void InitTextureTransparant();
      void InitShadow();
    };
  }
}