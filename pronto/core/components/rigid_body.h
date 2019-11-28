#pragma once
#include "core/component.h"
#include "core/type_id.h"
#include <glm/vec3.hpp>

class btRigidBody;

namespace pronto
{
  class RigidBody : public Component, public CompTID<RigidBody>
  {
  public:
    RigidBody(Entity* ent);

    void Update() override {};

    void SetMass(const float mass);

    void ApplyForce(const glm::vec3& force);
  private:
    btRigidBody* rigid_body_; //!< Local cache
  };
}
