#pragma once

#include <wtypes.h>
#define COM_NO_WINDOWS_H
#include "graphics/pc/d3dx12.h"
#undef COM_NO_WINDOWS_H
#include <Windows.h>
#include <map>
#include <string>
#include <d3d12shader.h>

namespace tinygltf
{
  struct Primitive;
  class Model;
}

namespace pronto
{
  namespace graphics
  {
    class D3D12Renderer;
  }

  class ModelResource;
  class ShaderResource;
  class MaterialResource;
  class MeshResource;

  class Resource
  {
  public:
    static Resource* Get();
    ~Resource();

    bool Init(graphics::D3D12Renderer* renderer);
    void Update();
    //Texture* ImportImage(const LPCWSTR file);
    ModelResource* LoadModel(const std::string& file);
    ShaderResource* LoadShader(
      const std::string& file,
      D3D12_SHADER_VERSION_TYPE type);

    MaterialResource* LoadMaterial(const std::string& file);
    MaterialResource* LoadMaterialFromGltf(
      const std::string& file,
      const int material_index,
      const tinygltf::Model& model);

    MeshResource* LoadMeshFromGltf(
        const std::string& file,
        const tinygltf::Primitive& primitive,
        const tinygltf::Model& model);

  private:
    static Resource* muh_instance_;

    Resource();
    Resource(const Resource&) : 
      renderer_(nullptr)
    {}
  
    graphics::D3D12Renderer* renderer_;
    std::map<std::string, MeshResource*> meshes_;
    std::map<std::string, ModelResource*> models_;
    std::map<std::string, ShaderResource*> shaders_;
    std::map<std::string, MaterialResource*> materials_;
  };
}
