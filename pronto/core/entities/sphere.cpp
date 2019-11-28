#include "Sphere.h"
#include "core/world.h"
#include "core/application.h"
#include "core/components/model.h"
#include "platform/resource.h"
#include "platform/renderer.h"
#include "core/components/fps_controller.h"
#include "core/components/point_light.h"
#include <glm/gtc/quaternion.hpp>


namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  Sphere::Sphere(World* world) :
    Entity(world),
    model_(nullptr)
  {
    model_ = AddComponent<Model>();
    model_->SetModel("resource/sphere/scene.gltf");
    transform()->SetScale(glm::vec3(0.05f));

    AddComponent<PointLight>();
  }

  void Sphere::Update()
  {
    Entity::Update();
  }
}
