#pragma once
#include <string>
#include <chrono>
#include <set>
#include <map>
#include <vector>
#include <tuple>

#include <glm/mat4x4.hpp>
#include <d3d12.h>

#include <tiny_gltf.h>
#pragma warning(push, 3) 
#include "d3dx12.h"
#pragma warning(pop) 
#include "d3d12_command_queue.h"
#include "core/type_id.h"
#include "d3d12_image.h"

#include "d3d12_constant_buffer_heap.h"
#include "d3d12_state_object.h"

#include <wrl.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct IDXGISwapChain4;
struct IDXGIAdapter4;

typedef unsigned int UINT; //winnt fwd
typedef void *HANDLE; //winnt fwd
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t; //stdint fwd
typedef float FLOAT;

struct HWND__;
typedef struct HWND__ *HWND;

struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;

namespace pronto
{
  class Camera;
  class World;
  class ModelResource;
  class Transform;
  class Image;
  class PointLight;
  class DirectionalLight;
  class Entity;
  class Model;

  namespace graphics
  {
    class PCWindow;
    class DescriptorAllocator;
    class CommandList;

    class D3D12Renderer
    {
    public:
      D3D12Renderer();
      ~D3D12Renderer();

      bool Init(HINSTANCE inst, World* world);
      bool Closed();
      void Update();
      void RenderUpdate();
      void Render();
      void Close();

      PCWindow* window();

      void Resize(int width, int height);

      void RegisterModel(Model* model);
      //void UnregisterModel(const std::string& path, Transform* transform);
      void RegisterPointLight(PointLight* light);
      void UnRegisterPointLight(PointLight* light);
      //void RegisterDirectionalLight(DirectionalLight* light);
      //void UnRegisterDirectionalLight(DirectionalLight* light);

      void main_cam(Camera* cam);


    private:
      std::vector<Entity*> entities_;

      std::set<PointLight*> point_lights_;
      std::set<DirectionalLight*> directional_lights_;

      std::string vertex_shader_;
      std::string pixel_shader_;

      PCWindow* window_;

      Camera* main_cam_;
      World* world_;

      CommandQueue* direct_cq_;
      CommandQueue* copy_cq_;

      unsigned int width_;
      unsigned int height_;

      const static uint8_t num_frames_ = 3;
      bool use_warp_;

      ID3D12Device2* device_;
      IDXGISwapChain4* swap_chain_;

      Microsoft::WRL::ComPtr<ID3D12Resource> back_buffers_[num_frames_];
     
      ID3D12DescriptorHeap* rtv_descriptor_heap_;
      UINT rtv_descriptor_size_;
      UINT current_bb_index_;

      uint64_t fence_values_[num_frames_];
      uint64_t frame_values_[num_frames_];

      bool vsync_ = true;
      bool tearing_supported_ = false;
      bool full_screen_ = false;


      // Depth buffer.
      ID3D12Resource* depth_buffer_;
      // Descriptor heap for depth buffer.
      ID3D12DescriptorHeap* dsv_heap_;

      D3D12_VIEWPORT viewport_;
      D3D12_RECT scissor_rect_;

      float fov_;

      glm::mat4 view_matrix_;
      glm::mat4 projection_matrix_;

      //ShadowMapPass* shadow_map_pass_;

      StateObject state_object_;

      std::unique_ptr<DescriptorAllocator> descriptor_allocators_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];


      bool MakeWindow(const std::string& name, HINSTANCE inst);

      bool InitPipeline();

      bool CheckTearingSupport();

      IDXGIAdapter4* GetAdapter(bool use_warp);

      ID3D12Device2* CreateDevice(IDXGIAdapter4* adapter);

      IDXGISwapChain4* CreateSwapChain(
        HWND hwnd,
        ID3D12CommandQueue* command_queue,
        uint32_t width,
        uint32_t height,
        uint32_t buffer_count);

      ID3D12DescriptorHeap* CreateDescriptorHeap(
        ID3D12Device2* device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t num_descriptors);

      void UpdateRenderTargetViews();

      HANDLE CreateEventHandle();


      void TransitionResource(
        ID3D12GraphicsCommandList* command_list,
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES before_state,
        D3D12_RESOURCE_STATES after_state);

      void ClearRTV(
        ID3D12GraphicsCommandList* command_list,
        D3D12_CPU_DESCRIPTOR_HANDLE rtv,
        FLOAT* clear_color);

      void ClearDepth(
        ID3D12GraphicsCommandList* command_list,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv,
        FLOAT depth = 1.0f);

      void ResizeDepthBuffer(int width, int height);

      void Flush();
    };
  }
}
