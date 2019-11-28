#pragma once

#include "core/type_id.h"

#include <map>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#pragma warning(push, 3) 
#include <btBulletDynamicsCommon.h>
#pragma warning(pop) 

// TODO: This class is not cross platform (map)
namespace pronto
{
  class Transform;

  class PhysicsWorld
  {
  public:
    PhysicsWorld();
    ~PhysicsWorld();

    void Init();

    btRigidBody* SpawnRigidBody(float mass, const Transform* trans);

    btRigidBody* SpawnStaticConvexHull(
      const glm::mat4& model_matrix,
      const glm::mat4& trans,
      const glm::vec3* vertices, 
      size_t vert_count);

    void Step();

  private:
    btDiscreteDynamicsWorld* dynamics_world_;
    //btAlignedObjectArray<btCollisionShape*> collision_shapes_;
    btBroadphaseInterface* broadphase_;
    btCollisionDispatcher* dispatcher_;
    btConstraintSolver* solver_;
    btDefaultCollisionConfiguration* collision_configuration_;
    //TODO: use tuple or simular. In theory a entity may only have one rigid body. 
    // However, one entity could be owner of the map, which will contain many rigidbodies.
    // Unless using a compound model / bind more than one collider to a rigidbody
    std::map<EID, btRigidBody*> rigid_bodies_;
  };
}
