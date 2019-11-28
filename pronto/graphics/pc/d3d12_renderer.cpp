#pragma once

#include "d3d12_renderer.h"
#include "d3d12_upload_buffer.h"
#include "d3d12_shadow_map.h"
#include "pc_window.h"
#include "platform/resource.h"
#include "core/components/model.h"
#include "core/components/mesh.h"
#include "core/components/material.h"
#include "core/components/camera.h"
#include "core/components/point_light.h"
#include "core/components/directional_light.h"

#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//-=-=-=-=-
#include <inspectable.h>
#include <Robuffer.h>
//-=-=-=-=-=-=-
#include <dxgi1_6.h>
#include <d3dcompiler.h> 



// STL Headers
#include <algorithm> 
#include <cassert> 
#include <stdint.h>

#include <algorithm> // For std::min and std::max.

#include <iostream>

#include <windef.h>

#include "core/world.h"
#include "core/entities/box.h"

#include "platform/resource.h"
#include "platform/conversions.h"

#include "d3d12_image.h"

#include "d3d12_dynamic_descriptor_heap.h"
#include "d3d12_descriptor_allocator.h"
#include "d3d12_command_list.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "resource/pc/material_resource.h"
#include "resource/pc/mesh_resource.h"
#include "resource/pc/model_resource.h"


//TODO: Clean up includes
//TODO: Make use of the upload buffer
//TODO: Remove or move functionality from this god class

namespace pronto
{
  namespace graphics
  {
    //---------------------------------------------------------------------------------------------
    D3D12Renderer::D3D12Renderer() :
      entities_(std::vector<Entity*>()),
      window_(nullptr),
      main_cam_(nullptr),
      world_(nullptr),
      direct_cq_(nullptr),
      copy_cq_(nullptr),
      width_(1440),
      height_(900),
      use_warp_(false),
      device_(nullptr),
      swap_chain_(nullptr),
      //back_buffers_(new ID3D12Resource*[num_frames_]),
      rtv_descriptor_heap_(nullptr),
      rtv_descriptor_size_(0),
      current_bb_index_(0),
      vsync_(true),
      tearing_supported_(false),
      full_screen_(false),
      depth_buffer_(nullptr),
      dsv_heap_(nullptr),
      viewport_(
        CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_))),
      scissor_rect_(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)),
      fov_(90)
      //shadow_map_pass_(nullptr)
    {
      //memset(back_buffers_, 0, sizeof(ID3D12Resource*) * num_frames_);
    }

    //---------------------------------------------------------------------------------------------
    D3D12Renderer::~D3D12Renderer()
    {
    }

    //---------------------------------------------------------------------------------------------
    bool D3D12Renderer::Init(HINSTANCE inst, World* world)
    {
      if (MakeWindow("pronto", inst) == false)
      {
        assert(false && "Failed to make a window...");
        return false;
      }

      if (InitPipeline() == false)
      {
        assert(false && "Failed to setup pipeline...");
        return false;
      }

      window_->Show();
      world_ = world;

      return true;
    }

    //---------------------------------------------------------------------------------------------
    bool D3D12Renderer::MakeWindow(const std::string& name, HINSTANCE inst)
    {
      if (window_ != nullptr)
      {
        assert(false && "You already created a window you dummy!");
        return false;
      }

      window_ = new PCWindow(this);

      if (window_->Make(width_, height_, 400, 200, name, inst) == false)
      {
        return false;
      }

      return true;
    }

    //---------------------------------------------------------------------------------------------
    bool D3D12Renderer::InitPipeline()
    {
      if (window_ == nullptr)
      {
        assert(false && "Be sure to initialize the window beforehand!");
        return false;
      }

      SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

      // Enable the debug layer
#ifdef _DEBUG
      ID3D12Debug* debug_interface;
      if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
      {
        debug_interface->EnableDebugLayer();
        ID3D12Debug1* di_1;
        debug_interface->QueryInterface(&di_1);
        di_1->SetEnableGPUBasedValidation(TRUE); // This is slow, yo
      }
#endif

      tearing_supported_ = CheckTearingSupport();

      IDXGIAdapter4* dxgi_adapter4 = GetAdapter(use_warp_);

      device_ = CreateDevice(dxgi_adapter4);

      direct_cq_ = new CommandQueue(device_, D3D12_COMMAND_LIST_TYPE_DIRECT);
      copy_cq_ = new CommandQueue(device_, D3D12_COMMAND_LIST_TYPE_COPY);

      swap_chain_ = CreateSwapChain(
        window_->h_wnd_,
        direct_cq_->command_q(),
        width_,
        height_,
        num_frames_);

      current_bb_index_ = swap_chain_->GetCurrentBackBufferIndex();

      rtv_descriptor_heap_ = 
        CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, num_frames_);

      rtv_descriptor_size_ =
        device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

      UpdateRenderTargetViews();

      for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
      {
        descriptor_allocators_[i] = std::make_unique<DescriptorAllocator>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i), device_);
      }

      // Create the descriptor heap for the depth-stencil view.
      D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
      dsv_heap_desc.NumDescriptors = 8;
      dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
      dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      assert(SUCCEEDED(
        device_->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&dsv_heap_))));

      dsv_heap_->SetName(L"dsv heap");

      state_object_ = StateObject(device_);

      // Resize/Create the depth buffer.
      ResizeDepthBuffer(width_, height_);

      // Loading default material
      auto* cl = direct_cq_->GetCommandList();
      Resource::Get()->LoadMaterial("default")->Upload(cl);
      auto fence = direct_cq_->ExecuteCommandList(cl);
      direct_cq_->WaitForFenceValue(fence);

      //shadow_map_pass_ = new ShadowMapPass(this, device_, dsv_heap_);
      //shadow_map_pass_->Init();

      return true;
    }

    //---------------------------------------------------------------------------------------------
    bool D3D12Renderer::Closed()
    {
      return window_ == nullptr ? false : window_->Closed();
    }

    //---------------------------------------------------------------------------------------------
    PCWindow* D3D12Renderer::window()
    {
      return window_;
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::Resize(int width, int height)
    {
      // Update the client size.
      if (width_ != width || height_ != height)
      {
        width_ = std::max(1, width);
        height_ = std::max(1, height);

        for (int i = 0; i < num_frames_; ++i)
        {
          direct_cq_->WaitForFenceValue(fence_values_[i]);
        }

        // Does a flush
        ResizeDepthBuffer(width, height);

        for (int i = 0; i < num_frames_; ++i)
        {
          back_buffers_[i].Reset();
        }

        rtv_descriptor_heap_->Release();

        DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
        assert(SUCCEEDED(swap_chain_->GetDesc(&swap_chain_desc)));

      //https://docs.microsoft.com/en-us/windows/win32/api/d3d12sdklayers/nf-d3d12sdklayers-id3d12debugdevice-reportlivedeviceobjects

        assert(SUCCEEDED(
          swap_chain_->ResizeBuffers(
            num_frames_,
            width_,
            height_,
            swap_chain_desc.BufferDesc.Format,
            swap_chain_desc.Flags)));

        current_bb_index_ = swap_chain_->GetCurrentBackBufferIndex();

        CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, num_frames_);

        UpdateRenderTargetViews();

        viewport_ =
          CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_));
      }
    }

    //---------------------------------------------------------------------------------------------
    // Perhaps Model should be called modelcomponent :think:
    void D3D12Renderer::RegisterModel(Model* model)
    {
      if (model->model_resource()->is_uploaded() == false)
      {
        CommandList* command_list = direct_cq_->GetCommandList();
        ID3D12GraphicsCommandList* direct_cl = command_list->command_list();
        direct_cl->SetName(L"register model");

        model->model_resource()->Upload(command_list);

        auto fence = direct_cq_->ExecuteCommandList(command_list);
        direct_cq_->WaitForFenceValue(fence);
      }

      auto children = model->transform()->GetChildren(true);

      for (auto* child : children)
      {
        if (child->entity()->GetComponent<Mesh>() != nullptr)
        {
          entities_.push_back(child->entity());
        }
      }

      std::sort(entities_.begin(), entities_.end(), 
        [](const auto& lhs, const auto& rhs)
        {
          Material* lhs_mat = lhs->GetComponent<Material>();
          Material* rhs_mat = rhs->GetComponent<Material>();

          if (lhs_mat != nullptr && rhs_mat != nullptr)
          {
            return lhs_mat->material_resource()->type() < rhs_mat->material_resource()->type();
          }

          return false;
        });
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::Update()
    {
      window()->Poll();
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::RenderUpdate()
    {
      static uint64_t frame_counter = 0;
      static double elapsed_seconds = 0.0;
      static double time_since_start = 0.0;
      static std::chrono::high_resolution_clock clock;
      static auto t0 = clock.now();

      frame_counter++;
      auto t1 = clock.now();
      std::chrono::duration<double, std::milli> delta_time = t1 - t0;
      t0 = t1;
      time_since_start += delta_time.count();
      std::chrono::duration<double, std::ratio<1, 1>> delta_seconds = delta_time;
      elapsed_seconds += delta_seconds.count();

      if (elapsed_seconds > 0.5)
      {
        char buffer[500];
        auto fps = frame_counter / elapsed_seconds;
        sprintf_s(buffer, 500, "MS: %f MS2: %f FPS: %f\n", delta_time.count(), Timer::delta_time(), fps);
        OutputDebugString(buffer);

        frame_counter = 0;
        elapsed_seconds = 0.0;

        // hackhackhackhackhack
        Resource::Get()->Update();
      }

      view_matrix_ = main_cam_->GetViewMatrix();

      // Update the projection matrix.
      main_cam_->height(height_);
      main_cam_->width(width_);
      main_cam_->fov(fov_);
      main_cam_->near_plane(0.1f);
      main_cam_->far_plane(1000.f);

      projection_matrix_ = main_cam_->GetProjectionMatrix();
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::Render()
    {
      CommandList* command_list = direct_cq_->GetCommandList();
      ID3D12GraphicsCommandList* graphics_cl = command_list->command_list();
      graphics_cl->SetName(L"Render command list");
      
      CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        rtv_descriptor_heap_->GetCPUDescriptorHandleForHeapStart(),
        current_bb_index_,
        rtv_descriptor_size_);

      D3D12_CPU_DESCRIPTOR_HANDLE dsv = dsv_heap_->GetCPUDescriptorHandleForHeapStart();

      // Clear the render targets.
      {
        TransitionResource(
          graphics_cl,
          back_buffers_[current_bb_index_].Get(),
          D3D12_RESOURCE_STATE_PRESENT,
          D3D12_RESOURCE_STATE_RENDER_TARGET);

        //FLOAT clear_color[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        //FLOAT clear_color[] = { 253.f / 255.f, 255.f / 255.f, 212.f / 255.f, 1.0f };
        FLOAT clear_color[] = { 255.f / 255.f, 225.f / 255.f, 156.f / 255.f, 1.0f };
       
        ClearRTV(graphics_cl, rtv, clear_color);
        ClearDepth(graphics_cl, dsv);
      }

      graphics_cl->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

      graphics_cl->RSSetViewports(1, &viewport_);
      graphics_cl->RSSetScissorRects(1, &scissor_rect_);

      //// Shadow realm
      //{
      //  shadow_map_pass_->Render(
      //    directional_light_,
      //    command_list,
      //    resources_on_gpu_,
      //    to_draw_);
      //}

      //// Directional light
      //{
      //  glm::mat4 blyat[3] =
      //  {
      //    directional_light_->transform()->ModelMatrix(),
      //    shadow_map_pass_->view_matrix(),
      //    shadow_map_pass_->projection_matrix()
      //  };
      //  constant_buffer_heap_.Write(
      //    blyat,
      //    sizeof(ModelViewProjection),
      //    graphics_cl,
      //    6);
      //  TransitionResource(
      //    graphics_cl,
      //    shadow_map_pass_->depth_buffer(),
      //    D3D12_RESOURCE_STATE_DEPTH_WRITE,
      //    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      //  auto allocation =
      //    descriptor_allocators_[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->Allocate(1);
      //D3D12_SHADER_TYPE_DESC
      //  D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
      //  desc.Format = DXGI_FORMAT_R32_FLOAT;
      //  desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      //  desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      //  desc.Texture2D.MipLevels = 1;
      //  device_->CreateShaderResourceView(
      //    shadow_map_pass_->depth_buffer(), &desc, allocation.GetDescriptorHandle());
      //  command_list->descriptor_heap()->StageDescriptors(
      //    5, 0, 1, allocation.GetDescriptorHandle());
      //  if (directional_light_ != nullptr)
      //  {
      //    static auto boi = directional_light_->CreateStruct();
      //    boi = directional_light_->CreateStruct();
      //    graphics_cl->SetGraphicsRoot32BitConstants(
      //      4, sizeof(DirectionalLightBuffer) / 4, &boi, 0);
      //  }
      //}

      //Perhaps this should be cached...
      std::vector<PointLightBuffer> lights;
      lights.reserve(point_lights_.size());

      for (const auto& light : point_lights_)
      {
        lights.push_back(light->CreateStruct());
        lights.rbegin()->view_pos_ = view_matrix_ * lights.rbegin()->world_pos_;
      }

      UploadBuffer::Allocation point_lights_heapAllocation;

      if (lights.size() > 0)
      {
        size_t buffer_size = lights.size() * sizeof(PointLightBuffer);

        point_lights_heapAllocation =
          command_list->upload_buffer()->Allocate(buffer_size, sizeof(PointLightBuffer));

        memcpy(point_lights_heapAllocation.CPU, lights.data(), buffer_size);
      }

      // Draw things :D
      // Gather all entities
      // Sort them based on their rendering needs (material (texture, bump map, transparant, etc)
      // Draw them based on a hash pso (sulphur)
      
      // Todo: I should prob just store all the thingies on the register call, shouldn't i?
      // It would ensure that I would only draw things that can be drawn
      // REMEMBA TO SET MODEL MATROX (up there where I also set the view and projection, maybe put those somewhere else :think:
      for (auto* ent : entities_)
      {
        auto mesh_comp = ent->GetComponent<Mesh>();
        auto material_comp = ent->GetComponent<Material>();

        if (material_comp != nullptr)
        {
          auto material_resource = material_comp->material_resource();

          //if (material_resource->type() != PrimitiveType::kColor)
          //{
          //  continue;
          //}

          auto* pso = state_object_.PSO(material_resource->type());

          assert(pso != nullptr && "I don't think we have the right pso m8");

          graphics_cl->SetPipelineState(pso);

          // TODO: Set this dynamically based upon the current pso
          command_list->SetGraphicsRootSignature(
            state_object_.RootSig(material_resource->type()),
            state_object_.RootSigDesc(material_resource->type()).Desc_1_1,
            8, // Bit mask  
            1);

          switch (material_resource->type())
          {
          case PrimitiveType::kColor:
          {
            //Num of lights
            auto size = (UINT)point_lights_.size();
            graphics_cl->SetGraphicsRoot32BitConstant(4, size, 0);

            if (size > 0)
            {
              graphics_cl->SetGraphicsRootShaderResourceView(5, point_lights_heapAllocation.GPU);
            }

            //Base Color
            graphics_cl->SetGraphicsRoot32BitConstants(
              3, sizeof(glm::vec4) / 4, &material_resource->base_color().color_.data, 0);

            FLOAT boi[] = { 1.f, 1.f, 1.f, 1.f };
            graphics_cl->OMSetBlendFactor(boi);
          }
          break;
          case PrimitiveType::kTexture:
          {
            //TODO: Check out maybe I can use different thingyies here, the ones in the command_list for example
            auto allocation =
              descriptor_allocators_[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->Allocate(1);

            // If I recall correctly, the resource view desc is extracted from the resource
            device_->CreateShaderResourceView(
              material_resource->texture().resource_, nullptr, allocation.GetDescriptorHandle());

            command_list->descriptor_heap()->StageDescriptors(
              3, 0, 1, allocation.GetDescriptorHandle());

            command_list->descriptor_heap()->
              CommitStagedDescriptorsForDraw(command_list->command_list());

            //Num of lights
            auto size = (UINT)point_lights_.size();
            graphics_cl->SetGraphicsRoot32BitConstant(4, size, 0);

            if (size > 0)
            {
              graphics_cl->SetGraphicsRootShaderResourceView(5, point_lights_heapAllocation.GPU);
            }
          }
          break;
          case PrimitiveType::kTransparantTexture:
          {
            //TODO: Check out maybe I can use different thingyies here, the ones in the command_list for example
            auto allocation =
              descriptor_allocators_[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->Allocate(1);

            // If I recall correctly, the resource view desc is extracted from the resource
            device_->CreateShaderResourceView(
              material_resource->texture().resource_, nullptr, allocation.GetDescriptorHandle());

            command_list->descriptor_heap()->StageDescriptors(
              3, 0, 1, allocation.GetDescriptorHandle());

            command_list->descriptor_heap()->
              CommitStagedDescriptorsForDraw(command_list->command_list());

            //Num of lights
            auto size = (UINT)point_lights_.size();
            graphics_cl->SetGraphicsRoot32BitConstant(4, size, 0);

            if (size > 0)
            {
              graphics_cl->SetGraphicsRootShaderResourceView(5, point_lights_heapAllocation.GPU);
            }

            FLOAT boi[] = { 1.f, 1.f, 1.f, 1.f };
            graphics_cl->OMSetBlendFactor(boi);
          }
          break;
          default:
            assert(false && "Do you want to get fekt?");
            break;
          }
        }
        else
        {
          assert(false && "but why?");
        }

        if (mesh_comp != nullptr)
        {
          auto mesh_resource = mesh_comp->mesh_resource();

          auto& modelmatrix = ent->transform()->ModelMatrix(Space::kWorld);
          graphics_cl->SetGraphicsRoot32BitConstants(
            0, sizeof(glm::mat4) / 4, &modelmatrix, 0);
          graphics_cl->SetGraphicsRoot32BitConstants(
            1, sizeof(glm::mat4) / 4, &view_matrix_, 0);
          graphics_cl->SetGraphicsRoot32BitConstants(
            2, sizeof(glm::mat4) / 4, &projection_matrix_, 0);

          D3D12_VERTEX_BUFFER_VIEW buffers[3] = {};

          if (mesh_resource->positions().is_valid() == true)
          {
            buffers[0] = mesh_resource->positions().vbv_;
          }

          if (mesh_resource->normals().is_valid() == true)
          {
            buffers[1] = mesh_resource->normals().vbv_;
          }

          if (mesh_resource->tex_coords().is_valid() == true)
          {
            buffers[2] = mesh_resource->tex_coords().vbv_;
          }

          graphics_cl->IASetVertexBuffers(0, 3, buffers);

          graphics_cl->IASetPrimitiveTopology(mesh_resource->indices().topology_);
          graphics_cl->IASetIndexBuffer(&mesh_resource->indices().ibv_);

          graphics_cl->DrawIndexedInstanced((UINT)mesh_resource->indices().count_, 1, 0, 0, 0);
        }
        else
        {
          assert(false && "Theoretically this really shouldn't happen yo");
        }
      }

      // Present
      {
        TransitionResource(
          graphics_cl,
          back_buffers_[current_bb_index_].Get(),
          D3D12_RESOURCE_STATE_RENDER_TARGET,
          D3D12_RESOURCE_STATE_PRESENT);

        uint64_t fence = direct_cq_->ExecuteCommandList(command_list);

        fence_values_[current_bb_index_] = fence;
        frame_values_[current_bb_index_] = Timer::frame_count();

        UINT sync_interval = vsync_ ? 1 : 0;
        UINT present_flags = tearing_supported_ && !vsync_ ? DXGI_PRESENT_ALLOW_TEARING : 0;
        assert(SUCCEEDED(swap_chain_->Present(sync_interval, present_flags)));

        current_bb_index_ = swap_chain_->GetCurrentBackBufferIndex();

        direct_cq_->WaitForFenceValue(fence_values_[current_bb_index_]);

        for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
          descriptor_allocators_[i]->ReleaseStaleDescriptors(frame_values_[current_bb_index_]);
        }
      }
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::Close()
    {
      direct_cq_->Close();
      copy_cq_->Close();
    }

    //void D3D12Renderer::UnregisterModel(const std::string& path, Transform* transform)
    //{
    //  if (to_draw_.count(path) != 0)
    //  {
    //    auto& v = to_draw_.at(path);
    //    v.erase(std::remove(v.begin(), v.end(), transform));
    //  }
    //}

    void D3D12Renderer::RegisterPointLight(PointLight* light)
    {
      point_lights_.emplace(light);
    }

    void D3D12Renderer::UnRegisterPointLight(PointLight* light)
    {
      point_lights_.erase(light);
    }

    //void D3D12Renderer::RegisterDirectionalLight(DirectionalLight* light)
    //{
    //  directional_light_ = light;
    //}

    //void D3D12Renderer::UnRegisterDirectionalLight(DirectionalLight* light)
    //{
    //  if (directional_light_ == light)
    //  {
    //    directional_light_ = nullptr;
    //  }
    //}

    //void D3D12Renderer::UpdateShaders()
    //{
    //  // Make this pretty, alright? Alright.
    //  if (Resource::Get()->LoadShader(pixel_shader_)->FileModified()
    //    || Resource::Get()->LoadShader(vertex_shader_)->FileModified())
    //  {
    //    Resource::Get()->LoadShader(pixel_shader_)->UpdateBuffer();
    //    Resource::Get()->LoadShader(vertex_shader_)->UpdateBuffer();
    //    LoadContent();
    //  }
    //}

    void D3D12Renderer::main_cam(Camera* cam)
    {
      main_cam_ = cam;
    }

    //---------------------------------------------------------------------------------------------
    bool D3D12Renderer::CheckTearingSupport()
    {
      BOOL allow_tearing = FALSE;

      IDXGIFactory4* factory4;
      if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
      {
        IDXGIFactory5* factory5;
        if (SUCCEEDED(factory4->QueryInterface(&factory5)))
        {
          if (FAILED(factory5->CheckFeatureSupport(
            DXGI_FEATURE_PRESENT_ALLOW_TEARING,
            &allow_tearing,
            sizeof(allow_tearing))))
          {
            allow_tearing = FALSE;
          }
        }
      }
      return allow_tearing == TRUE;
    }


    //---------------------------------------------------------------------------------------------
    IDXGIAdapter4* D3D12Renderer::GetAdapter(bool use_warp)
    {
      IDXGIFactory4* dxgi_factory;
      UINT create_factory_flags = 0;
#if defined(_DEBUG)
      create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

      assert(
        SUCCEEDED(
          CreateDXGIFactory2(
            create_factory_flags, 
            IID_PPV_ARGS(&dxgi_factory))
          )
        && "Failed to make dxgi factory");

      IDXGIAdapter1* dxgi_adapter1;
      IDXGIAdapter4* dxgi_adapter4 = nullptr;

      if (use_warp)
      {
        assert(SUCCEEDED(dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter1)))
          && "Failed to get wrap adapter");
        dxgi_adapter1->QueryInterface(&dxgi_adapter4);
      }
      else
      {
        SIZE_T max_dedicated_video_memory = 0;
        for (UINT i = 0; dxgi_factory->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
          DXGI_ADAPTER_DESC1 dxgi_adapter_desc1;
          dxgi_adapter1->GetDesc1(&dxgi_adapter_desc1);

          // Check to see if the adapter can create a D3D12 DEVICE without actually 
          // creating it. The adapter with the largest dedicated video memory
          // is favored.
          if ((dxgi_adapter_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
            SUCCEEDED(
              D3D12CreateDevice(
                dxgi_adapter1,
                D3D_FEATURE_LEVEL_11_0,
                __uuidof(ID3D12Device),
                nullptr))
            &&
            dxgi_adapter_desc1.DedicatedVideoMemory > max_dedicated_video_memory)
          {
            max_dedicated_video_memory = dxgi_adapter_desc1.DedicatedVideoMemory;
            SUCCEEDED(dxgi_adapter1->QueryInterface(&dxgi_adapter4));
          }
        }
      }

      assert(dxgi_adapter4 != nullptr
        && "Failed to get adapter");

      return dxgi_adapter4;
    }

    //---------------------------------------------------------------------------------------------
    ID3D12Device2* D3D12Renderer::CreateDevice(IDXGIAdapter4* adapter)
    {
      ID3D12Device2* d3d12_device2;

      if (FAILED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12_device2))))
      {
        assert(false && "Failed to create device!");
        return nullptr;
      }

#ifdef _DEBUG
      ID3D12InfoQueue* info_queue;

      if (SUCCEEDED(d3d12_device2->QueryInterface(&info_queue)))
      {
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_MESSAGE, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY categories[] = {};

        // Suppress messages based on their severity level
        //D3D12_MESSAGE_SEVERITY severities[] = {};

        // Suppress individual messages by their ID
        //D3D12_MESSAGE_ID deny_ids[] = {};

        //D3D12_INFO_QUEUE_FILTER new_filter = {};
        //NewFilter.DenyList.NumCategories = sizeof(categories) / sizeof(categories[0]);
        //NewFilter.DenyList.pCategoryList = categories;
        //new_filter.DenyList.NumSeverities = sizeof(severities) / sizeof(severities[0]);
        //new_filter.DenyList.pSeverityList = severities;
        //new_filter.DenyList.NumIDs = sizeof(deny_ids) / sizeof(deny_ids[0]);
        //new_filter.DenyList.pIDList = deny_ids;

        //SUCCEEDED(info_queue->PushStorageFilter(&new_filter));
      }
#endif

      return d3d12_device2;
    }

    //---------------------------------------------------------------------------------------------
    IDXGISwapChain4* D3D12Renderer::CreateSwapChain(
      HWND hwnd,
      ID3D12CommandQueue* command_queue,
      uint32_t width,
      uint32_t height,
      uint32_t buffer_count)
    {
      IDXGISwapChain4* dxgi_swap_chain4;
      IDXGIFactory4* dxgi_factory4;
      UINT create_factory_flags = 0;

#ifdef _DEBUG
      create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

      assert(SUCCEEDED(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory4))));

      DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
      swap_chain_desc.Width = width;
      swap_chain_desc.Height = height;
      swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      swap_chain_desc.Stereo = FALSE;
      swap_chain_desc.SampleDesc = { 1, 0 };
      swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swap_chain_desc.BufferCount = buffer_count;
      swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
      swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
      // It is recommended to always allow tearing if tearing support is available.
      swap_chain_desc.Flags = tearing_supported_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
      IDXGISwapChain1* dxgi_swap_chain1;

      assert(SUCCEEDED(dxgi_factory4->CreateSwapChainForHwnd(
        command_queue,
        hwnd,
        &swap_chain_desc,
        nullptr,
        nullptr,
        &dxgi_swap_chain1)));

      // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
      // will be handled manually.
      assert(SUCCEEDED(dxgi_factory4->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)));

      assert(SUCCEEDED(dxgi_swap_chain1->QueryInterface(&dxgi_swap_chain4)));

      return dxgi_swap_chain4;
    }

    //---------------------------------------------------------------------------------------------
    ID3D12DescriptorHeap* D3D12Renderer::CreateDescriptorHeap(
      ID3D12Device2* device,
      D3D12_DESCRIPTOR_HEAP_TYPE type,
      uint32_t num_descriptors)
    {
      ID3D12DescriptorHeap* descriptor_heap;

      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = num_descriptors;
      desc.Type = type;

      assert(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap))));
      descriptor_heap->SetName(L"created desciptor heap");
      return descriptor_heap;
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::UpdateRenderTargetViews()
    {
      CD3DX12_CPU_DESCRIPTOR_HANDLE 
        rtv_handle(rtv_descriptor_heap_->GetCPUDescriptorHandleForHeapStart());

      for (int i = 0; i < num_frames_; ++i)
      {
        Microsoft::WRL::ComPtr<ID3D12Resource> back_buffer;
        assert(SUCCEEDED(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&back_buffer))));

        device_->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_handle);

        back_buffer->SetName(L"Back buffer lyf"); 

        back_buffers_[i] = back_buffer;

        rtv_handle.Offset(rtv_descriptor_size_);
      }
    }

    ////---------------------------------------------------------------------------------------------
    //HANDLE D3D12Renderer::CreateEventHandle()
    //{
    //  HANDLE fence_event;

    //  fence_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    //  if (fence_event == NULL)
    //  {
    //    assert("Failed to create fence event.");
    //    DWORD error = ::GetLastError();
    //  }


    //  return fence_event;
    //}

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::TransitionResource(
      ID3D12GraphicsCommandList* command_list,
      ID3D12Resource* resource,
      D3D12_RESOURCE_STATES before_state, 
      D3D12_RESOURCE_STATES after_state)
    {
      CD3DX12_RESOURCE_BARRIER barrier =
        CD3DX12_RESOURCE_BARRIER::Transition(
        resource,
          before_state,
          after_state);

      command_list->ResourceBarrier(1, &barrier);
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::ClearRTV(
      ID3D12GraphicsCommandList* command_list,
      D3D12_CPU_DESCRIPTOR_HANDLE rtv,
      FLOAT* clear_color)
    {
      command_list->ClearRenderTargetView(rtv, clear_color, 0, nullptr);
    }

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::ClearDepth(
      ID3D12GraphicsCommandList* command_list,
      D3D12_CPU_DESCRIPTOR_HANDLE dsv,
      FLOAT depth)
    {
      command_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
    }

    ////---------------------------------------------------------------------------------------------
    //void D3D12Renderer::UpdateBufferResource(
    //  ID3D12GraphicsCommandList* command_list,
    //  const std::wstring& name,
    //  ID3D12Resource** destination_resource,
    //  ID3D12Resource** intermediate_resource,
    //  size_t num_elements,
    //  size_t element_size,
    //  const void* buffer_data,
    //  D3D12_RESOURCE_FLAGS flags,
    //  D3D12_RESOURCE_DESC* desc)
    //{
    //  size_t buffer_size = element_size * num_elements;

    //  UINT64 tex_upload_buffer_size;

    //  if (desc != nullptr)
    //  {
    //    // this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    //    // each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    //    // eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
    //    //textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    //    device_->GetCopyableFootprints(
    //      desc,
    //      0,
    //      1,
    //      0,
    //      nullptr,
    //      nullptr,
    //      nullptr,
    //      &tex_upload_buffer_size);
    //  }

    //  // Create a committed resource for the GPU resource in a default heap.
    //  assert(SUCCEEDED(
    //    device_->CreateCommittedResource(
    //      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    //      D3D12_HEAP_FLAG_NONE,
    //      desc == nullptr ? &CD3DX12_RESOURCE_DESC::Buffer(buffer_size, flags) : desc,
    //      D3D12_RESOURCE_STATE_COMMON,
    //      nullptr,
    //      IID_PPV_ARGS(destination_resource))));

    //  // Create an committed resource for the upload.
    //  if (buffer_data != nullptr)
    //  {
    //    command_list->ResourceBarrier(
    //      1,
    //      &CD3DX12_RESOURCE_BARRIER::Transition(
    //        *destination_resource, 
    //        D3D12_RESOURCE_STATE_COMMON,
    //        D3D12_RESOURCE_STATE_COPY_DEST));

    //    assert(SUCCEEDED(
    //      device_->CreateCommittedResource(
    //        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    //        D3D12_HEAP_FLAG_NONE,
    //        &CD3DX12_RESOURCE_DESC::Buffer(desc == nullptr ? buffer_size : tex_upload_buffer_size),
    //        D3D12_RESOURCE_STATE_GENERIC_READ,
    //        nullptr,
    //        IID_PPV_ARGS(intermediate_resource))));

    //    // Slice is for 3D textures' total size (or 1D or 2D total size)
    //    D3D12_SUBRESOURCE_DATA subresource_data = { 0 };
    //    subresource_data.pData = buffer_data;
    //    subresource_data.RowPitch = desc == nullptr ? buffer_size : num_elements;
    //    subresource_data.SlicePitch = 
    //      desc == nullptr ? subresource_data.RowPitch : num_elements * desc->Height;

    //    UpdateSubresources(
    //      command_list,
    //      *destination_resource,
    //      *intermediate_resource,
    //      0, // intermediate offset
    //      0, // first subresource
    //      1, // num subresources
    //      &subresource_data);

    //    intermediate_resource[0]->SetName(L"intermediate upload thingy");
    //    destination_resource[0]->SetName(name.c_str());
    //  }
    //}

    //---------------------------------------------------------------------------------------------
    void D3D12Renderer::ResizeDepthBuffer(int width, int height)
    {
      // Flush any GPU commands that might be referencing the depth buffer.
      Flush();

      width = std::max(1, width);
      height = std::max(1, height);

      // Resize screen dependent resources.
      // Create a depth buffer.
      D3D12_CLEAR_VALUE optimized_clear_value = {};
      optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
      optimized_clear_value.DepthStencil = { 1.0f, 0 };

      assert(SUCCEEDED(
        device_->CreateCommittedResource(
          &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
          D3D12_HEAP_FLAG_NONE,
          &CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT,
            width,
            height,
            1,
            0,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
          D3D12_RESOURCE_STATE_DEPTH_WRITE,
          &optimized_clear_value,
          IID_PPV_ARGS(&depth_buffer_))));

      depth_buffer_->SetName(L"depth buffer yo");

      // Update the depth-stencil view.
      D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
      dsv.Format = DXGI_FORMAT_D32_FLOAT;
      dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv.Texture2D.MipSlice = 0;
      dsv.Flags = D3D12_DSV_FLAG_NONE;

      device_->CreateDepthStencilView(
        depth_buffer_,
        &dsv,
        dsv_heap_->GetCPUDescriptorHandleForHeapStart());
    }

    void D3D12Renderer::Flush()
    {
      copy_cq_->Flush();
      direct_cq_->Flush();
    }
  }
}
