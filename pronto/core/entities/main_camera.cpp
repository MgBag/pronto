#include "main_camera.h"
#include "core/components/fps_controller.h"
#include "platform/input.h"
#include "core/world.h" 
#include "core/entities/box.h"
#include "core/entities/sphere.h"
#include "core/components/rigid_body.h"

namespace pronto
{
  MainCamera::MainCamera(World* world) :
    Entity(world)
  {
    cam_ = AddComponent<Camera>();
    AddComponent<FpsController>();
  }

  void MainCamera::Update()
  {
    Entity::Update();
  }

  Camera* MainCamera::cam() const
  {
    return cam_;
  }
}
