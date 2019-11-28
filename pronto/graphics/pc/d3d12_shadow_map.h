#pragma once

#include "platform/utils.h"

#pragma warning(push, 3) 
#include "d3dx12.h"
#pragma warning(pop) 
#include <map>
#include <vector>
#include <string>

#include <glm/mat4x4.hpp>

namespace tinygltf
{
  class Model;
}

namespace pronto
{
  class DirectionalLight;
  struct Renderable;
  class Transform;
  class Entity;

  namespace graphics
  {
    class D3D12Renderer;
    class CommandList;
    struct Renderable;
    class DescriptorAllocator;
    class StateObject;

    class ShadowMapPass
    {
    public:
      ShadowMapPass(
        D3D12Renderer* renderer,
        ID3D12Device2* device, 
        ID3D12DescriptorHeap* dsv_heap);

      ~ShadowMapPass();

      void Init();

      void Render(
        DirectionalLight* light, 
        CommandList* command_list,
        StateObject* state_object,
        const std::vector<Entity*>& entities_);

      const glm::mat4& projection_matrix();
      const glm::mat4& view_matrix();
      ID3D12Resource* depth_buffer();

    private:
      //D3D12Renderer* renderer_;
      //ID3D12PipelineState* pipeline_state_;
      ID3D12Resource* depth_buffer_;
      ID3D12Device2* device_;
      ID3D12DescriptorHeap* dsv_heap_;
      //ID3D12RootSignature* root_signature_;
      DescriptorAllocator* descriptor_allocator_;

      CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle_;
      //CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_description_;
      UINT srv_;
      std::string vertex_shader_path_;
      unsigned int dimention_;

      glm::mat4 view_matrix_;
      glm::mat4 projection_matrix_;

      void CreateShadowMapResource();
      void CreatePipelineState();
    };
  }
}
