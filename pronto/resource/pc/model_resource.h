#pragma once
#include <string>
#include <glm/mat4x4.hpp>
#include "utils/pc/ntree.h"
#include "graphics/enum.h"

namespace tinygltf
{
  class Model;
  struct Primitive;
}

namespace pronto
{
  namespace graphics
  {
    class CommandList;
  }

  class MeshResource;
  class MaterialResource;

  struct Primitive
  {
    Primitive() : 
      mesh_(nullptr),
      material_(nullptr),
      matrix_(glm::mat4(1))
    {};

    Primitive(MeshResource* mesh, MaterialResource* mat) :
      mesh_(mesh),
      material_(mat),
      matrix_(glm::mat4(1))
    {};

    MeshResource* mesh_;
    MaterialResource* material_;
    glm::mat4 matrix_;
  };

  class ModelResource
  {
  public:
    ModelResource(const std::string& path);
    ~ModelResource();

    bool Init();
    bool Upload(graphics::CommandList* command_list);
    Ntree<Primitive>& tree();

    const tinygltf::Model& model() const;
    const std::string& path() const;

    bool is_uploaded();

  private:
    bool is_init_;
    bool is_uploaded_;

    std::string path_;
    tinygltf::Model* model_;
    Ntree<Primitive> tree_;

    void SetModelNodes(
      const tinygltf::Model& model,
      const int node_index,
      Ntree<Primitive>& tree);

    void SetMatrix(
      const tinygltf::Model& model,
      const int node_index,
      Ntree<Primitive>& tree);

    void SetMesh(
      const tinygltf::Model& model,
      const int node_index,
      Ntree<Primitive>& tree);

    void SetPrimitive(
      const tinygltf::Model& model,
      const tinygltf::Primitive& primitive,
      const std::string& mesh_name,
      Ntree<Primitive>& tree);

    void SetMaterial(
      const int material_index,
      const tinygltf::Model& model,
      Ntree<Primitive>& tree);

    void Traverse(
      Ntree<Primitive>& tree,
      graphics::CommandList* command_list);

    void UploadToGPU(
      Primitive& primitive,
      graphics::CommandList* command_list);
  };
}
