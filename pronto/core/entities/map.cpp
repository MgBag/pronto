#include "map.h"
#include "core/world.h"
#include "core/application.h"
#include "core/components/model.h"
#include "physics/physics_world.h"
#include "platform/resource.h"
#include "platform/renderer.h"
#include "core/components/fps_controller.h"
#include "core/components/rigid_body.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  Map::Map(World* world) :
    Entity(world),
    model_(nullptr),
    phys_world_(world->physics_world())
  {
    model_ = AddComponent<Model>();

    //model_->SetModel("resource/restaurant/scene.gltf");
    model_->SetModel("resource/littlest/scene.gltf");
    //model_->SetModel("resource/sponza/scene.gltf");

    //transform_->SetScale(glm::vec3(0.01f));

    //ModelResource* map_resource = model_->model_resource();

    //const tinygltf::Model& model = map_resource->model();

    //for (auto& scene : model.scenes)
    //{
    //  for (auto& node : scene.nodes)
    //  {
    //    ItModelNodes(model, node, glm::identity<glm::mat4>());
    //  }
    //}
  }

  void Map::Update()
  {
    Entity::Update();
  }

  // bind models
  void Map::ItModelNodes(
    const tinygltf::Model& model,
    const int node_index,
    const glm::mat4& parent)
  {
    glm::mat4 matrix = glm::identity<glm::mat4>();
    CalculateNodeMatrix(model, node_index, matrix);
    matrix = parent * matrix;

    if (model.nodes[node_index].mesh != -1)
    {
      SpawnMesh(model, node_index, matrix);
    }

    for (auto& child_node_index : model.nodes[node_index].children)
    {
      ItModelNodes(model, child_node_index, matrix);
    }
  }

  void Map::SpawnMesh(
    const tinygltf::Model& model,
    const int node_index,
    const glm::mat4& matrix)
  {
    const tinygltf::Node& node = model.nodes[node_index];
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];

    for (auto& primitive : mesh.primitives)
    {
      for (auto& attrib : primitive.attributes)
      {

        if (attrib.first.compare("POSITION") == 0)
        {
          const tinygltf::Accessor& accessor = model.accessors[attrib.second];
          const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
          int byte_stride = accessor.ByteStride(buffer_view);

          assert(byte_stride != -1 && "somethings fucky");

          const unsigned char* vertices = 
            model.buffers[buffer_view.buffer].data.data() 
            + buffer_view.byteOffset + accessor.byteOffset;

          phys_world_->SpawnStaticConvexHull(
            matrix,
            transform_->ModelMatrix(Space::kWorld),
            (const glm::vec3*)vertices,
            accessor.count);
        }
      }
    }
  }

  void Map::CalculateNodeMatrix(
    const tinygltf::Model& model,
    const int node_index,
    glm::mat4& matrix)
  {
    // Calculate parent matrices here.
    const tinygltf::Node& node = model.nodes[node_index];
    // Upload offset 

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
      {
        (float)node.rotation[3],
        (float)node.rotation[0],
        (float)node.rotation[1],
        (float)node.rotation[2]
      };

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
}
