#pragma once
#include "core/entity.h"
#include <glm/mat4x4.hpp>

namespace tinygltf
{
  class Model;
}

namespace pronto
{
  class World;
  class Model;
  class PhysicsWorld;

  class Map : public Entity, public EntTID<Map>
  {
  public:
    Map(World* scene);
    virtual void Update() override;
  private:
    Model* model_;
    PhysicsWorld* phys_world_;

    void ItModelNodes(
      const tinygltf::Model& model,
      const int node_index,
      const glm::mat4& parent);

    void CalculateNodeMatrix(
      const tinygltf::Model& model,
      const int node_index,
      glm::mat4& matrix);

    void SpawnMesh(
      const tinygltf::Model& model,
      const int node_index,
      const glm::mat4& matrix);
  };
}
