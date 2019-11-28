#include "model_resource.h"
#include "utils/pc/conversions.h"
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assert.h>
#include "graphics/pc/d3d12_command_list.h"
#include "resource.h"
#include <tiny_gltf.h>
#include "mesh_resource.h"
#include "material_resource.h"
#include <string>

namespace pronto
{
  using namespace graphics;

  //-----------------------------------------------------------------------------------------------
  ModelResource::ModelResource(const std::string& path) :
    is_init_(false),
    is_uploaded_(false),
    path_(path),
    tree_(Ntree<Primitive>(Primitive(), "model:" + path))
  {
    std::string ext = path_.substr(path_.find_last_of(".") + 1);
    assert(ext == "gltf" || ext == "glb");
  }

  //-----------------------------------------------------------------------------------------------
  ModelResource::~ModelResource()
  {
  }

  //-----------------------------------------------------------------------------------------------
  bool ModelResource::Init()
  {
    if (is_init_ == true)
    {
      return true;
    }

    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool succes;

    model_ = new tinygltf::Model();
    
    if (path_.substr(path_.find_last_of(".") + 1) == "glb")
    {
      succes = loader.LoadBinaryFromFile(model_, &err, &warn, path_);
    }
    else
    {
      succes = loader.LoadASCIIFromFile(model_, &err, &warn, path_);
    }

    if (warn.empty() == false)
    {
      printf("Warn: %s\n", warn.c_str());
    }

    if (err.empty() == false)
    {
      printf("Err: %s\n", err.c_str());
      assert(false);
    }

    if (succes == false)
    {
      printf("Failed to parse glTF\n");
    }

    // Create model tree, but we spawn all scenes, which we might not want
    for (int i = 0; i < model_->scenes.size(); ++i)
    {
      auto& scene = model_->scenes[i];

      auto& tree = tree_.AddChild("scene:" + std::to_string(i));

      for (int j = 0; j < scene.nodes.size(); ++j)
      {
        auto& node = scene.nodes[j];
        SetModelNodes(*model_, node, tree.AddChild("node:" + std::to_string(j)));
      }
    }

    is_init_ = true;
    return succes;
  }

  // Uploads the model tree to the gpu
  //---------------------------------------------------------------------------------------------
  bool ModelResource::Upload(graphics::CommandList* command_list)
  {
    if (is_uploaded_ == false)
    {
      // Traverse tree
      // Per entry upload its members 
      Traverse(tree_, command_list);

      is_uploaded_ = true;
    }

    return true;
  }

  //---------------------------------------------------------------------------------------------
  Ntree<Primitive>& ModelResource::tree()
  {
    return tree_;
  }

  //---------------------------------------------------------------------------------------------
  void ModelResource::SetModelNodes(
    const tinygltf::Model& model,
    const int node_index,
    Ntree<Primitive>& tree)
  {
    SetMatrix(model, node_index, tree);

    if (model.nodes[node_index].mesh != -1)
    {
      SetMesh(model, node_index, tree);
    }

    for (int i = 0; i < model.nodes[node_index].children.size(); ++i)
    {
      auto& child_node_index = model.nodes[node_index].children[i];

      SetModelNodes(model, child_node_index, tree.AddChild("node:" + std::to_string(i)));
    }
  }

  //-----------------------------------------------------------------------------------------------
  void ModelResource::SetMatrix(
    const tinygltf::Model& model,
    const int node_index,
    Ntree<Primitive>& tree)
  {
    const tinygltf::Node& node = model.nodes[node_index];
    glm::mat4& matrix = tree.data().matrix_;

    if (node.matrix.size() != 0)
    {
      // Tiny gltf is in doubles....
      for (int i = 0; i < node.matrix.size(); ++i)
      {
        matrix[(int)(i / 4)][i % 4] = (float)node.matrix[i];
      }
    }

    if (node.translation.size() == 3)
    {
      glm::vec3 offset =
      {
        (float)node.translation[0],
        (float)node.translation[1],
        (float)node.translation[2]
      };

      matrix = matrix * glm::translate(glm::mat4(1.0f), offset);
    }

    if (node.rotation.size() == 4)
    {
      glm::quat rotation =
        glm::quat(
          (float)node.rotation[3],
          (float)node.rotation[0],
          (float)node.rotation[1],
          (float)node.rotation[2]);

      matrix = matrix * glm::mat4_cast(rotation);
    }

    if (node.scale.size() == 3)
    {
      glm::vec3 scale =
      {
        (float)node.scale[0],
        (float)node.scale[1],
        (float)node.scale[2]
      };

      matrix = matrix * glm::scale(scale);
    }
  }

  //-----------------------------------------------------------------------------------------------
  void ModelResource::SetMesh(
    const tinygltf::Model& model,
    const int node_index,
    Ntree<Primitive>& tree)
  {
    const tinygltf::Node& node = model.nodes[node_index];
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];

    for (int i = 0; i < mesh.primitives.size(); ++i)
    {
      auto& primitive = mesh.primitives[i];

      auto& subchild = tree.AddChild("primitive" + std::to_string(i));

      SetPrimitive(
        model,
        primitive,
        mesh.name + std::to_string(node.mesh) + ':' + std::to_string(i),
        subchild
      );

      //Material
      if (primitive.material != -1)
      {
        SetMaterial(primitive.material, model, subchild);
      }
    }   
  }

  //-----------------------------------------------------------------------------------------------
  void ModelResource::SetPrimitive(
    const tinygltf::Model& model,
    const tinygltf::Primitive& primitive,
    const std::string& mesh_name,
    Ntree<Primitive>& tree)
  {
    auto* mesh_resource = Resource::Get()->LoadMeshFromGltf(
      path_ + mesh_name,//std::to_string(primitive.indices),
      primitive,
      model
    );

    tree.data().mesh_ = mesh_resource;
  }

  //TODO: Figure out how uv and texutres are related > multiple uv
  // Isnt pixel_type the same as bits? but in bits, duh
  //-----------------------------------------------------------------------------------------------
  void ModelResource::SetMaterial(
    const int material_index,
    const tinygltf::Model& model,
    Ntree<Primitive>& tree)
  {
    auto* mat_resource = Resource::Get()->LoadMaterialFromGltf(
      path_ + ":mat:" + std::to_string(material_index),
      material_index,
      model
    );

    tree.data().material_ = mat_resource;
  }

  //-----------------------------------------------------------------------------------------------
  void ModelResource::Traverse(Ntree<Primitive>& tree, graphics::CommandList* command_list)
  {
    UploadToGPU(tree.data(), command_list);

    for (auto& node : tree.children())
    {
      Traverse(node, command_list);
    }
  }

  //-----------------------------------------------------------------------------------------------
  void ModelResource::UploadToGPU(Primitive& node, graphics::CommandList* command_list)
  {
    if (node.mesh_ != nullptr)
    {
      node.mesh_->Upload(command_list);
    }
    if (node.material_ != nullptr)
    {
      node.material_->Upload(command_list);
    }
  }

  //-----------------------------------------------------------------------------------------------
  const tinygltf::Model& ModelResource::model() const
  {
    return *model_;
  }

  //-----------------------------------------------------------------------------------------------
  const std::string& ModelResource::path() const
  {
    return path_;
  }

  //-----------------------------------------------------------------------------------------------
  bool ModelResource::is_uploaded()
  {
    return is_uploaded_;
  }
}
