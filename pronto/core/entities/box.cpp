#include "box.h"
#include "core/world.h"
#include "core/application.h"
#include "core/components/model.h"
#include "platform/resource.h"
#include "platform/renderer.h"
#include "core/components/fps_controller.h"
#include "core/components/rigid_body.h"
#include <glm/gtc/quaternion.hpp>


namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  Box::Box(World* world) :
    Entity(world),
    model_(nullptr)
  {
    model_ = AddComponent<Model>();
    
    model_->SetModel("resource/tex_box.gltf");
  }

  void Box::Update()
  {
    Entity::Update();
  }
}
