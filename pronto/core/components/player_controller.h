#pragma once
#include "core/component.h"
#include "core/type_id.h"

class btRigidBody;

namespace pronto
{
  class PlayerController : public Component, public CompTID<PlayerController>
  {
  public:
    PlayerController(Entity* ent);

    void Update() override {};

  private:
    btRigidBody* rigid_body_; //!< Local cache
  };
}
