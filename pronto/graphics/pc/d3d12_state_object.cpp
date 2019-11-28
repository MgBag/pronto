#include "d3d12_state_object.h"
#include "resource/pc/shader_resource.h"
#include "resource/pc/resource.h"

namespace pronto
{
  namespace graphics
  {
    StateObject::StateObject(ID3D12Device2* device) :
      device_(device),
      is_init_(std::vector<bool>((int)PrimitiveType::kCount)),
      pso_(std::vector<ID3D12PipelineState*>((int)PrimitiveType::kCount)),
      root_signature_(std::vector<ID3D12RootSignature*>((int)PrimitiveType::kCount)),
      root_signature_description_(
        std::vector<CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC>((int)PrimitiveType::kCount))
    {

    }

    ID3D12PipelineState* StateObject::PSO(PrimitiveType type)
    {
      switch (type)
      {
      case PrimitiveType::kColor:
      {
        if (is_init_[(int)PrimitiveType::kColor] == false)
        {
          InitColor();
        }

        return pso_[(int)PrimitiveType::kColor];
      }
      break;
      case PrimitiveType::kTexture:
      {
        if (is_init_[(int)PrimitiveType::kTexture] == false)
        {
          InitTexture();
        }

        return pso_[(int)PrimitiveType::kTexture];
      }
      break;
      case PrimitiveType::kTransparantTexture:
      {
        if (is_init_[(int)PrimitiveType::kTransparantTexture] == false)
        {
          InitTextureTransparant();
        }

        return pso_[(int)PrimitiveType::kTransparantTexture];
      }
      break;
      case PrimitiveType::kNormalMap:
        return nullptr;
        break;
      case PrimitiveType::kMultipleUV:
        return nullptr;
        break;
      case PrimitiveType::kCount:
        return nullptr;
        break;
      case PrimitiveType::kShadow:
      {
        if (is_init_[(int)PrimitiveType::kShadow] == false)
        {
          InitShadow();
        }

        return pso_[(int)PrimitiveType::kShadow];
      }
      break;
      default:
        return nullptr;
        break;
      }
    }

    ID3D12RootSignature* StateObject::RootSig(PrimitiveType type)
    {
      switch (type)
      {
      case PrimitiveType::kColor:
        {
          if (is_init_[(int)PrimitiveType::kColor] == false)
          {
            InitColor();
          }

          return root_signature_[(int)PrimitiveType::kColor];
        }
        break;
      case PrimitiveType::kTexture:
        {
          if (is_init_[(int)PrimitiveType::kTexture] == false)
          {
            InitTexture();
          }

          return root_signature_[(int)PrimitiveType::kTexture];
        }
        break;
      case PrimitiveType::kTransparantTexture:
        {
          if (is_init_[(int)PrimitiveType::kTransparantTexture] == false)
          {
            InitTextureTransparant();
          }

          return root_signature_[(int)PrimitiveType::kTransparantTexture];
        }
        break;
      case PrimitiveType::kNormalMap:
        return nullptr;
        break;
      case PrimitiveType::kMultipleUV:
        return nullptr;
        break;
      case PrimitiveType::kCount:
        return nullptr;
        break;
      case PrimitiveType::kShadow:
      {
        if (is_init_[(int)PrimitiveType::kShadow] == false)
        {
          InitShadow();
        }

        return root_signature_[(int)PrimitiveType::kShadow];
      }
      default:
        return nullptr;
        break;
      }
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& StateObject::RootSigDesc(PrimitiveType type)
    {
      static CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC noin;

      switch (type)
      {
      case PrimitiveType::kColor:
        {
          if (is_init_[(int)PrimitiveType::kColor] == false)
          {
            InitColor();
          }

          return root_signature_description_[(int)PrimitiveType::kColor];
        }
        break;
      case PrimitiveType::kTexture:
        {
          if (is_init_[(int)PrimitiveType::kTexture] == false)
          {
            InitTexture();
          }

          return root_signature_description_[(int)PrimitiveType::kTexture];
        }
        break;
      case PrimitiveType::kTransparantTexture:
        {
          if (is_init_[(int)PrimitiveType::kTransparantTexture] == false)
          {
            InitTextureTransparant();
          }

          return root_signature_description_[(int)PrimitiveType::kTransparantTexture];
        }
        break;
      case PrimitiveType::kNormalMap:
        return noin;
        break;
      case PrimitiveType::kMultipleUV:
        return noin;
        break;
      case PrimitiveType::kShadow:
      {
        if (is_init_[(int)PrimitiveType::kShadow] == false)
        {
          InitShadow();
        }

        return root_signature_description_[(int)PrimitiveType::kShadow];
      }
      case PrimitiveType::kCount:
        return noin;
        break;
      default:
        return noin;
        break;
      }
    }
  }
}
