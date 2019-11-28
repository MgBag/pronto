#include "d3d12_shadow_map.h"
#include "d3d12_command_list.h"
#include "d3d12_renderer.h"
#include "d3d12_descriptor_allocator.h"
#include "d3d12_dynamic_descriptor_heap.h"
#include "d3d12_state_object.h"
#include "platform/resource.h"
#include "core/components/directional_light.h"
#include "core/components/transform.h"
#include "core/entity.h"
#include "resource/pc/resource.h"
#include "resource/pc/shader_resource.h"
#include "resource/pc/model_resource.h"
#include "resource/pc/mesh_resource.h"
#include "resource/pc/material_resource.h"
#include <assert.h>
#include <d3dcompiler.h>
#include <glm/matrix.hpp>
#include <tiny_gltf.h>

namespace pronto
{
  namespace graphics
  {
    //---------------------------------------------------------------------------------------------
    ShadowMapPass::ShadowMapPass(
      D3D12Renderer* renderer,
      ID3D12Device2* device,
      ID3D12DescriptorHeap* dsv_heap) :
      device_(device),
      dsv_heap_(dsv_heap),
      vertex_shader_path_("shadow_vertex.hlsl"),
      descriptor_allocator_(nullptr),
      dimention_(8192),
      view_matrix_(glm::mat4(1)),
      projection_matrix_(glm::mat4(1))
    {
    }

    //---------------------------------------------------------------------------------------------
    ShadowMapPass::~ShadowMapPass()
    {
    }
   
    //---------------------------------------------------------------------------------------------
    void ShadowMapPass::Init()
    {
      CreateShadowMapResource();
      CreatePipelineState();

      descriptor_allocator_ =
        new DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, device_);

      projection_matrix_ = glm::orthoLH_ZO(
        64.f, -64.f, 64.f, -64.f, 0.1f, 1000.f);
    }

    //---------------------------------------------------------------------------------------------
    void ShadowMapPass::Render(
      DirectionalLight* light, 
      CommandList* command_list, 
      StateObject* state_object,
      const std::vector<Entity*>& entities_)
    {
      ID3D12GraphicsCommandList* graphics_cl = command_list->command_list();

      CD3DX12_RESOURCE_BARRIER barrier =
        CD3DX12_RESOURCE_BARRIER::Transition(
          depth_buffer_,
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
          D3D12_RESOURCE_STATE_DEPTH_WRITE);

      graphics_cl->ResourceBarrier(1, &barrier);

      auto* pso = state_object->PSO(PrimitiveType::kShadow);
      assert(pso != nullptr && "I don't think we have the right pso m8");
      graphics_cl->SetPipelineState(pso);

      // TODO: Set this dynamically based upon the current pso
      command_list->SetGraphicsRootSignature(
        state_object->RootSig(PrimitiveType::kShadow),
        state_object->RootSigDesc(PrimitiveType::kShadow).Desc_1_1,
        0, // Bit mask  
        1);

      graphics_cl->ClearDepthStencilView(dsv_handle_, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

      graphics_cl->OMSetRenderTargets(
        0, nullptr, FALSE, &dsv_handle_);

      CD3DX12_VIEWPORT viewport =
        CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(dimention_), static_cast<float>(dimention_));
      graphics_cl->RSSetViewports(1, &viewport);

      CD3DX12_RECT scissor_rect(0, 0, LONG_MAX, LONG_MAX);
      graphics_cl->RSSetScissorRects(1, &scissor_rect);

      //glm::mat4 rotate = glm::inverse(glm::mat4_cast(light->transform()->rotation()));
      //glm::mat4 translate =
      //  glm::translate(glm::identity<glm::mat4>(), -light->transform()->position());

      view_matrix_ = glm::lookAtLH(
        light->transform()->position(),
        light->transform()->position() + light->transform()->Forward(), 
        -light->transform()->Up());

      glm::mat4 boi[2] = { view_matrix_, projection_matrix_ };

      graphics_cl->SetGraphicsRoot32BitConstants(
        0, sizeof(glm::mat4) / 4 * 2, boi, 0);

      //DrawGeometry(command_list, resources, to_draw);
      
      //auto allocation = descriptor_allocator_->Allocate(1);
      //device_->CreateShaderResourceView(
        //depth_buffer_, nullptr, allocation.GetDescriptorHandle());
    }

    //---------------------------------------------------------------------------------------------
    const glm::mat4& ShadowMapPass::projection_matrix()
    {
      return projection_matrix_;
    }

    //---------------------------------------------------------------------------------------------
    const glm::mat4& ShadowMapPass::view_matrix()
    {
      return view_matrix_;
    }

    ID3D12Resource* ShadowMapPass::depth_buffer()
    {
      return depth_buffer_;
    }

    //---------------------------------------------------------------------------------------------
    void ShadowMapPass::CreateShadowMapResource()
    {
      auto tex_2d = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R32_TYPELESS,
        dimention_,
        dimention_,
        1,
        0,
        1,
        0,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

      D3D12_CLEAR_VALUE optimized_clear_value = {};
      optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
      optimized_clear_value.DepthStencil = { 1.0f, 0 };

      assert(SUCCEEDED(
        device_->CreateCommittedResource(
          &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
          D3D12_HEAP_FLAG_NONE,
          &tex_2d,
          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
          &optimized_clear_value,
          IID_PPV_ARGS(&depth_buffer_))));

      depth_buffer_->SetName(L"Depth buffer for shadow map");

      // Update the depth-stencil view.
      D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
      dsv.Format = DXGI_FORMAT_D32_FLOAT;
      dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv.Texture2D.MipSlice = 0;
      dsv.Flags = D3D12_DSV_FLAG_NONE;
      
      dsv_handle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsv_heap_->GetCPUDescriptorHandleForHeapStart());

      dsv_handle_.Offset(
        1, device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

      device_->CreateDepthStencilView(
        depth_buffer_,
        &dsv,
        dsv_handle_);
    }

    //void ShadowMapPass::DrawGeometry(
    //  CommandList* command_list,
    //  const std::map<std::string, Renderable>& resources,
    //  const std::map<std::string, std::vector<Transform*>>& to_draw)
    //{
    //  for (auto const& draw : to_draw)
    //  {
    //    if (draw.second.size() != 0)
    //    {
    //      std::vector<glm::mat4> instances;
    //      instances.reserve(draw.second.size());

    //      for (auto const& transform : draw.second)
    //      {
    //        instances.push_back(transform->ModelMatrix());
    //      }

    //      // Upload instanced data here 
    //      ID3D12Resource* intermediate_buffer;
    //      ID3D12Resource* buffer;

    //      renderer_->UpdateBufferResource(
    //        command_list->command_list(),
    //        L"Instance " + std::wstring(draw.first.begin(), draw.first.end()),
    //        &buffer,
    //        &intermediate_buffer,
    //        draw.second.size(),
    //        sizeof(glm::mat4),
    //        instances.data(),
    //        D3D12_RESOURCE_FLAG_NONE);

    //      command_list->command_list()->ResourceBarrier(
    //        1,
    //        &CD3DX12_RESOURCE_BARRIER::Transition(
    //          buffer,
    //          D3D12_RESOURCE_STATE_COPY_DEST,
    //          D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    //      D3D12_VERTEX_BUFFER_VIEW instances_view;
    //      instances_view.BufferLocation = buffer->GetGPUVirtualAddress();
    //      instances_view.SizeInBytes = (UINT)(sizeof(glm::mat4) * draw.second.size());
    //      instances_view.StrideInBytes = (UINT)sizeof(glm::mat4);

    //      const Renderable& renderable = resources.at(draw.first);
    //      const tinygltf::Model& model = renderable.model_resource->model();

    //      for (auto& scene : model.scenes)
    //      {
    //        for (auto& node : scene.nodes)
    //        {
    //          DrawNodes(model, node, renderable, instances_view, command_list);
    //        }
    //      }
    //    }
    //  }
    //}

    //void ShadowMapPass::DrawNodes(
    //  const tinygltf::Model& model, 
    //  const int node,
    //  const Renderable& renderable, 
    //  const D3D12_VERTEX_BUFFER_VIEW& instances,
    //  CommandList* command_list)
    //{
    //  if (model.nodes[node].mesh != -1)
    //  {
    //    DrawMesh(model, node, renderable, instances, command_list);
    //  }

    //  for (auto& child_node : model.nodes[node].children)
    //  {
    //    DrawNodes(model, child_node, renderable, instances, command_list);
    //  }
    //}

    //void ShadowMapPass::DrawMesh(
    //  const tinygltf::Model& model, 
    //  const int node_index, 
    //  const Renderable& renderable, 
    //  const D3D12_VERTEX_BUFFER_VIEW& instances, 
    //  CommandList* command_list)
    //{
    //  const tinygltf::Node& node = model.nodes[node_index];
    //  const tinygltf::Mesh& mesh = model.meshes[node.mesh];
    //  auto cl = command_list->command_list();

    //  for (auto& primitive : mesh.primitives)
    //  {
    //    D3D12_VERTEX_BUFFER_VIEW buffers[3] = {};

    //    for (auto& attrib : primitive.attributes)
    //    {
    //      const tinygltf::Accessor& accessor = model.accessors[attrib.second];
    //      const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];

    //      int byte_stride = accessor.ByteStride(buffer_view);
    //      assert(byte_stride != -1 && "somethings fucky");
    //      DXGI_FORMAT format = pronto::ToDXGIFormat(accessor.type, accessor.componentType);

    //      if (attrib.first.compare("POSITION") == 0)
    //      {
    //        buffers[0] = renderable.vbvs.at(attrib.second);
    //      }
    //    }

    //    buffers[1] = renderable.matrix_views.at(node_index);
    //    buffers[2] = instances;

    //    cl->IASetVertexBuffers(0, 3, buffers);
    //    cl->IASetPrimitiveTopology(toPrimitiveTopology(primitive.mode));
    //    cl->IASetIndexBuffer(&renderable.ibvs.at(primitive.indices));

    //    const tinygltf::Accessor& indices_accessor = model.accessors[primitive.indices];
    //    cl->DrawIndexedInstanced((UINT)indices_accessor.count, instances.SizeInBytes / (UINT)sizeof(glm::mat4), 0, 0, 0);
    //  }
    //}
  }
}
