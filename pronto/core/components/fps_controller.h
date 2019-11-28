#pragma once
#include "core/component.h"
#include "core/type_id.h"
#include "glm/vec2.hpp"

namespace pronto
{
  class FpsController : public Component, public CompTID<FpsController>
  {
  public:
    FpsController(Entity* ent);
    ~FpsController();

    void Update() override;

  private:
    float yaw_;
    float pitch_;
    glm::vec2 last_mouse_;
  };
}
