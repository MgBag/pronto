#include "resource.h"
#include "model_resource.h"
#include "shader_resource.h"
#include "material_resource.h"
#include "mesh_resource.h"
#include "utils/pc/conversions.h"
#include "graphics/enum.h"
#include <d3d12.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <tiny_gltf.h>

namespace pronto
{
  Resource* Resource::muh_instance_ = nullptr;

  //-----------------------------------------------------------------------------------------------
  Resource::Resource() :
    renderer_(nullptr)
  {
  }

  //-----------------------------------------------------------------------------------------------
  Resource * Resource::Get()
  {
    if (muh_instance_ == nullptr)
    {
      muh_instance_ = new Resource();
    }

    return muh_instance_;
  }

  //-----------------------------------------------------------------------------------------------
  bool Resource::Init(graphics::D3D12Renderer* renderer)
  {
    renderer_ = renderer;

    return true;
  }

  void Resource::Update()
  {
    for (auto& shader : shaders_)
    {
      if (shader.second->FileModified())
      {
        shader.second->UpdateShader();
      }
    }
  }

  //-----------------------------------------------------------------------------------------------
  ShaderResource* Resource::LoadShader(
    const std::string& file,
    D3D12_SHADER_VERSION_TYPE type)
  {
    assert(file != "" && "pls no empty");
    if (shaders_.count(file) != 0)
    {
      return shaders_.at(file);
    }

    ShaderResource* shader = new ShaderResource(file, type);
    shaders_.emplace(file, shader);

    return shader;
  }

  //-----------------------------------------------------------------------------------------------
  MaterialResource* Resource::LoadMaterial(const std::string& file)
  {
    assert(file != "" && "pls no empty");
    if (materials_.count(file) != 0)
    {
      return materials_.at(file);
    }

    if (file == "default")
    {
      MaterialResource* default_color = new MaterialResource();
      default_color->base_color().color_ = glm::vec4(1,0,1,1);
      default_color->base_color().is_valid_ = true;
      default_color->type(graphics::PrimitiveType::kColor);

      materials_.emplace(file, default_color);

      return default_color;
    }

    return nullptr;
  }

  //-----------------------------------------------------------------------------------------------
  ModelResource* Resource::LoadModel(const std::string& file)
  {
    assert(file != "" && "pls no empty");
    if (models_.count(file) != 0)
    {
      return models_.at(file);
    }

    ModelResource* model = new ModelResource(file);
    model->Init();
    models_.emplace(file, model);

    return model;
  }

  //-----------------------------------------------------------------------------------------------
  MaterialResource* Resource::LoadMaterialFromGltf(
    const std::string& file,
    const int material_index, 
    const tinygltf::Model& model)
  {
    assert(file != "" && "pls no empty");
    printf(file.c_str());
    if (materials_.count(file) != 0)
    {
      return materials_.at(file);
    }

    MaterialResource* material_resource = new MaterialResource();
    material_resource->path(file);

    const tinygltf::Material& material = model.materials[material_index];

    for (const auto& val : material.values)
    {
      // TODO: Material can have both basecolortexture and basecolorfactor (and more, muh pbr)
      if (val.first.compare("baseColorTexture") == 0)
      {
        //TODO: Add multiple UV support
        //TODO: Look here for normalmap

        const tinygltf::Texture& texture =
          model.textures[(size_t)val.second.json_double_value.at("index")];
        const tinygltf::Image& image = model.images[texture.source];

        // So the actual buffer is already mapped?
        if (true)//image.bufferView == -1)
        {
          DXGI_FORMAT dxgi_format =
            ToDXGIFormat(image.component, image.pixel_type, true /*normalized*/);
          assert(dxgi_format != DXGI_FORMAT_UNKNOWN);

          ImageBuffer& texture = material_resource->texture();
          texture.buffer_ = image.image.data();
          texture.width_ = image.width;
          texture.height_ = image.height;
          texture.channel_depth_ = image.bits;
          texture.component_ = image.component;
          texture.count_ = image.image.size();
          texture.stride_ = image.pixel_type;
          texture.format_ = dxgi_format;

          if (material.alphaMode.compare("OPAQUE") == 0)
          {
            material_resource->type(graphics::PrimitiveType::kTexture);
          }
          else if (material.alphaMode.compare("BLEND") == 0)
          {
            material_resource->type(graphics::PrimitiveType::kTransparantTexture);
          }
          else if (material.alphaMode.compare("MASK") == 0)
          {
            // Mask orsmth maybe?
            material_resource->type(graphics::PrimitiveType::kTransparantTexture);
          }
        }
        else
        {
          assert(false && "Wait, no buffer?");
        }
      }
      else if (val.first.compare("baseColorFactor") == 0)
      {
        BaseColor& color = material_resource->base_color();

        color.color_ = ToVec4(val.second.number_array);
        color.is_valid_ = true;

        material_resource->type(graphics::PrimitiveType::kColor);
      }
      else if (val.first.compare("metallicFactor") == 0)
      {
        // TODO: Muh pbr
      }
      else if (val.first.compare("roughnessFactor") == 0)
      {
        // TODO: Muh pbr
      }
      else if (val.first.compare("metallicRoughnessTexture") == 0)
      {
        // TODO: Muh pbr 
      }
      else
      {
        assert(false && "Implement loading image from buffer or uri");
      }
    }

    materials_.emplace(file, material_resource);
    return material_resource;
  }

  //-----------------------------------------------------------------------------------------------
  MeshResource* Resource::LoadMeshFromGltf(
    const std::string& file,
    const tinygltf::Primitive& primitive, 
    const tinygltf::Model& model)
  {
    assert(file != "" && "pls no empty");

    if (meshes_.count(file) != 0)
    {
      return meshes_.at(file);
    }

    MeshResource* mesh_resource = new MeshResource();
    mesh_resource->path(file);
    // Attributes
    for (auto& attrib : primitive.attributes)
    {
      auto& semantic = attrib.first;
      auto& semantic_accessor = attrib.second;
      
      const tinygltf::Accessor& accessor = model.accessors[semantic_accessor];
      const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];

      int byte_stride = accessor.ByteStride(buffer_view);
      assert(byte_stride != -1 && "somethings fucky");
      assert(byte_stride < 2048 && "somethings fucky");
      DXGI_FORMAT format = ToDXGIFormat(accessor.type, accessor.componentType);

      if (semantic.compare("POSITION") == 0)
      {
        mesh_resource->positions(
          MeshBuffer(model.buffers[buffer_view.buffer].data.data() 
            + buffer_view.byteOffset
            + accessor.byteOffset, 
            byte_stride, 
            accessor.count, 
            format));
      }
      else if (semantic.compare("NORMAL") == 0)
      {
          mesh_resource->normals(
            MeshBuffer(model.buffers[buffer_view.buffer].data.data()
              + buffer_view.byteOffset
              + accessor.byteOffset,
              byte_stride,
              accessor.count,
              format));
      }
      else if (semantic.compare("TEXCOORD_0") == 0)
      {
          mesh_resource->tex_coords(
            MeshBuffer(model.buffers[buffer_view.buffer].data.data()
              + buffer_view.byteOffset
              + accessor.byteOffset,
              byte_stride,
              accessor.count,
              format));
      }
    }

    // Indices
    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
    int byte_stride = accessor.ByteStride(buffer_view);
    assert(byte_stride != -1 && "somethings fucky");
    assert(byte_stride < 2048 && "somethings fucky");

    IndexBuffer& indices_buffer = mesh_resource->indices();
    indices_buffer.buffer_ =
      model.buffers[buffer_view.buffer].data.data()
      + buffer_view.byteOffset
      + accessor.byteOffset;
    indices_buffer.stride_ = byte_stride;
    indices_buffer.count_ = accessor.count;
    indices_buffer.topology_ = toPrimitiveTopology(primitive.mode);
    indices_buffer.format_ = ToDXGIFormat(accessor.type, accessor.componentType);
    // Dx does not like 8 bit length index buffer
    if (indices_buffer.format_ == DXGI_FORMAT::DXGI_FORMAT_R8_UINT)
    {
      assert(byte_stride == 1);

      unsigned short* buffer = (unsigned short*)malloc(accessor.count * 2);
      for (int i = 0; i < accessor.count; ++i)
      {
        buffer[i] = (unsigned short)indices_buffer.buffer_[i];
      }

      indices_buffer.buffer_ = (unsigned char*)buffer;
      indices_buffer.format_ = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
      indices_buffer.stride_ = 2;
    }

    //Index of the renderable's views should now mirror the data in tinygltf
    //TODO: ADD EMPTY DATA FOR WHEN THERE IS NO NORMAL OR TEX COORD
    //Jam in some empty data, maybe I should have a buffer stored somewhere that is just 0? 
    //Consult.

    meshes_.emplace(file, mesh_resource);
    return mesh_resource;
  }
}
