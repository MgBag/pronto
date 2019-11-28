#include "Cylinder.h"
#include "core/world.h"
#include "core/application.h"
#include "core/components/model.h"
#include "platform/resource.h"
#include "platform/renderer.h"
#include "core/components/fps_controller.h"
#include "core/components/directional_light.h"
#include <glm/gtc/quaternion.hpp>

namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  Cylinder::Cylinder(World* world) :
    Entity(world),
    model_(nullptr)
  {
    model_ = AddComponent<Model>();
    model_->SetModel("resource/cylinder/scene.gltf");
    transform()->SetScale(glm::vec3(0.05f));
    AddComponent<DirectionalLight>();
  }

  void Cylinder::Update()
  {
    Entity::Update();
  }
}
