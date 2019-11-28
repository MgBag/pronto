#include "fps_controller.h"
#include "platform/input.h"
#include "core/entity.h"
#include "core/components/transform.h"
#include "platform/utils.h"
#include "core/world.h"
#include "core/entities/box.h"
#include "core/entities/truck.h"
#include "core/entities/sphere.h"
#include "core/components/rigid_body.h"
#include "core/entities/cylinder.h"
#include "core/components/model.h"
#include <glm/gtc/quaternion.hpp>

namespace pronto
{
  FpsController::FpsController(Entity* ent) :
    Component(ent),
    yaw_(0.f),
    pitch_(0.f),
    last_mouse_(glm::vec2())
  {
  }

  FpsController::~FpsController()
  {
  }

  void FpsController::Update()
  {
    Input& in = Input::GetInstance();
    glm::vec2 boi = in.GetMouseDelta();
    glm::vec2 dulta = last_mouse_ - in.GetMouse();
    Transform* trans = transform();
    static float sensitivity = 0.08f;
    static float base_speed = 0.025f;

    // Used for spot or directional lights
    //if (in.GetKey(Input::Keys::kC) > 0)
    //{
    //  Cylinder* cyl = world()->GetEntity<Cylinder>();
    //  if (cyl == nullptr)
    //  {
    //    cyl = world()->CreateEntity<Cylinder>();
    //  }
    //  cyl->transform()->SetRotation(trans->rotation());
    //  cyl->transform()->SetPosition(trans->position());
    //}

    //if (in.GetKey(Input::Keys::kR) > 0)
    //{
    //  for (auto& sphere : world()->GetEntities(Sphere::tid()))
    //  {
    //    sphere->Delete();
    //  }
    //}
    
    if (in.GetKey(Input::Keys::kOne) == 2)
    {
      auto box = world()->CreateEntity<Box>();
      box->transform()->SetPosition(transform()->position());
    }

    if (in.GetKey(Input::Keys::kTwo) == 2)
    {
      auto box = world()->CreateEntity<Truck>();
      box->transform()->SetPosition(transform()->position());
    }

    if (in.GetKey(Input::Keys::kThree) == 2)
    {
      Sphere* light = world()->CreateEntity<Sphere>();
      light->transform()->SetPosition(transform()->position());
    }

    if (in.GetKey(Input::Keys::kOEMMinus) == 2)
    {
      sensitivity -= 0.01f;
    }

    if (in.GetKey(Input::Keys::kOEMEquals) == 2)
    {
      sensitivity += 0.01f;
    }

    if (in.GetKey(Input::Keys::kOEMSqrOpen) == 2)
    {
      base_speed -= 0.005f;
    }

    if (in.GetKey(Input::Keys::kOEMSqrClose) == 2)
    {
      base_speed += 0.005f;
    }

    if (in.GetMouseButton(Input::MouseButton::kRightButton) > 0)
    {
      glm::vec2 mouse_delta = dulta;

      yaw_ -= mouse_delta.x * sensitivity;
      pitch_ -= mouse_delta.y * sensitivity;

      if (pitch_ > 89.0f) { pitch_ = 89.0f; }
      if (pitch_ < -89.0f) { pitch_ = -89.0f; }

      float yaw_radians = glm::radians(yaw_);
      float pitch_radians = glm::radians(pitch_);

      glm::vec3 target;
      target.x = sin(yaw_radians) * cos(pitch_radians);
      target.y = sin(pitch_radians);
      target.z = cos(yaw_radians) * cos(pitch_radians);
      glm::vec3 up;
      pitch_radians += glm::half_pi<float>();
      up.x = sin(yaw_radians) * cos(pitch_radians);
      up.y = sin(pitch_radians);
      up.z = cos(yaw_radians) * cos(pitch_radians);

      trans->SetRotation(glm::quatLookAtLH(target, up));
    }

    float speed = base_speed * Timer::delta_time_f();

    if (in.GetKey(Input::Keys::kShift) > 0)
    {
      speed *= 10;
    }
    if (in.GetKey(Input::Keys::kControl) > 0)
    {
      speed /= 10.f;
    }

    if (in.GetKey(Input::Keys::kW) > 0)
    {
      trans->TranslateBy(glm::vec3(0, 0, 1) * speed, Space::kLocal);
    }
    if (in.GetKey(Input::Keys::kS) > 0)
    {
      trans->TranslateBy(glm::vec3(0, 0, -1) * speed, Space::kLocal);
    }
    if (in.GetKey(Input::Keys::kA) > 0)
    {
      trans->TranslateBy(glm::vec3(-1, 0, 0) * speed, Space::kLocal);
    }
    if (in.GetKey(Input::Keys::kD) > 0)
    {
      trans->TranslateBy(glm::vec3(1, 0, 0) * speed, Space::kLocal);
    }
    if (in.GetKey(Input::Keys::kSpace) > 0)
    {
      trans->TranslateBy(glm::vec3(0, 1, 0) * speed, Space::kLocal);
    }

    if (in.GetKey(Input::Keys::kUp) > 0)
    {
      trans->RotateBy(glm::vec3(1, 0, 0), -0.1f * sensitivity, Space::kLocal);
    }
    if (in.GetKey(Input::Keys::kDown) > 0)
    {
      trans->RotateBy(glm::vec3(1, 0, 0), 0.1f * sensitivity, Space::kLocal);
    }
    if (in.GetKey(Input::Keys::kLeft) > 0)
    {
      trans->RotateBy(glm::vec3(0, 1, 0), -0.1f * sensitivity, Space::kWorld);
    }
    if (in.GetKey(Input::Keys::kRight) > 0)
    {
      trans->RotateBy(glm::vec3(0, 1, 0), 0.1f * sensitivity, Space::kWorld);
    }

    last_mouse_ = in.GetMouse();
  }
}
