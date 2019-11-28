#include "physics_world.h"
#include "core/components/transform.h"
#include "core/entity.h"
#include "platform/conversions.h"
#include "platform/utils.h"

// TODO: Fix naming and such

namespace pronto
{
  PhysicsWorld::PhysicsWorld() : 
    dynamics_world_(nullptr),
    broadphase_(nullptr),
    dispatcher_(nullptr),
    solver_(nullptr),
    collision_configuration_(nullptr)
  {
  }

  PhysicsWorld::~PhysicsWorld()
  {
  }

  void PhysicsWorld::Init()
  {
    collision_configuration_ = new btDefaultCollisionConfiguration();
    //m_collisionConfiguration->setConvexConvexMultipointIterations();

    dispatcher_ = new btCollisionDispatcher(collision_configuration_);

    broadphase_ = new btDbvtBroadphase();

    solver_ = new btSequentialImpulseConstraintSolver;

    dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collision_configuration_);

    dynamics_world_->setGravity(btVector3(0, -10, 0));
  }

  btRigidBody* PhysicsWorld::SpawnRigidBody(float mass, const Transform* trans)
  {
    btCollisionShape* shape = new btBoxShape(GlmToB(trans->scale() * 0.5f));

    btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
      shape->calculateLocalInertia(mass, localInertia);

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

    btTransform bt_trans;
    bt_trans.setOrigin(GlmToB(trans->position()));
    bt_trans.setRotation(GlmToB(trans->rotation()));

    btDefaultMotionState* myMotionState = new btDefaultMotionState(bt_trans);

    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

    btRigidBody* body = new btRigidBody(cInfo);
    //body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);
    //btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
    //body->setWorldTransform(startTransform);

    //TODO: Make compuid and set here
    body->setUserIndex(trans->entity()->eid());
    body->setUserPointer((void*)trans);
    rigid_bodies_.emplace(trans->entity()->eid(), body);
    dynamics_world_->addRigidBody(body);
    return body;
  }

  btRigidBody* PhysicsWorld::SpawnStaticConvexHull(
    const glm::mat4& model_matrix,
    const glm::mat4& trans, 
    const glm::vec3* vertices, 
    size_t vert_count)
  {
    btTransform bt_trans = GlmToB(trans);

    auto a = bt_trans.getOrigin();

    // No need for indices since a convex hull will use all its outlayers
    btConvexHullShape* shape = new btConvexHullShape();
    
    for (size_t i = 0; i < vert_count; ++i)
    {
      glm::vec4 point = model_matrix * glm::vec4(vertices[i], 1.f);

      shape->addPoint(GlmToB(point), false);
    }

    shape->recalcLocalAabb();

    //TODO: Optimize the way that this data is passed, perhaps use the method in the renderer
    btDefaultMotionState* motion_state = new btDefaultMotionState(bt_trans);

    btRigidBody::btRigidBodyConstructionInfo cInfo(0, motion_state, shape, { 0,0,0 });
    btRigidBody* body = new btRigidBody(cInfo);
    dynamics_world_->addRigidBody(body);

    return body;
  }

  void PhysicsWorld::Step()
  {
    //TODO: fix dt hacks
    dynamics_world_->stepSimulation((btScalar)0.016f);

    //Update Bodies
    for (auto pair : rigid_bodies_)
    {
      btMotionState* body = pair.second->getMotionState();
      Transform* trans = (Transform*)pair.second->getUserPointer();
      btTransform bt_trans;
      body->getWorldTransform(bt_trans);

      trans->SetPosition(BToGlm(bt_trans.getOrigin()));
      trans->SetRotation(BToGlm(bt_trans.getRotation()));
    }
  }
}
