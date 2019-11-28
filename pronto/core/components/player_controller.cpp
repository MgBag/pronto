#include "player_controller.h"
#include "core/entity.h"
#include "core/world.h"

#pragma warning(push, 3) 
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

namespace pronto
{
  PlayerController::PlayerController(Entity* ent) :
    Component(ent)
  {
    PhysicsWorld* phys_world = ent->world()->physics_world();
    rigid_body_ = phys_world->SpawnRigidBody(10, ent->transform());
  }
  /*
  void RigidBody::SetMass(const float mass)
  {
    btVector3 local_inertia(0, 0, 0);
    if (rigid_body_->getCollisionShape() != nullptr &&
      rigid_body_->getCollisionShape()->getShapeType() !=
      BroadphaseNativeTypes::EMPTY_SHAPE_PROXYTYPE)
    {
      rigid_body_->getCollisionShape()->calculateLocalInertia(mass, local_inertia);
    }
    rigid_body_->setMassProps(mass, local_inertia);
    rigid_body_->updateInertiaTensor();
    rigid_body_->activate(true);
  }*/
}
