#include "world.h"
#include "application.h"
#include "platform/utils.h"

namespace pronto
{
  World::World(Application* app) :
    application_(app),
    ents_(pvector<pvector<Entity*>>(EntTIDBase::size())),
    phys_world_(pnew<PhysicsWorld>())
  { 
    phys_world_->Init();
  }

  void World::Update()
  {
    // Delete entities
    UpdateDelete();

    // Update
    for (auto& el : ents_)
    {
      for (auto& e : el)
      {
        e->Update();
      }
    }

    static double passed = 0.0;
    passed += Timer::delta_time();
    if (passed > 16.0)
    {
      phys_world_->Step();
      passed = 0.0;
    }
  }

  void World::DeleteEntity(TID id, int i)
  {
    if (i == -1)
    {
      auto ent = ents_[id].rbegin();
      delete *ent;

      ents_[id].pop_back();
    }
    else if (i < (int)ents_[id].size())
    {
      ents_[id].erase(ents_[id].begin() + i);
    }
  }

  void World::DeleteEntity(EID id)
  {
    for (auto& el : ents_)
    {
      for (auto& e : el)
      {

        if (e->eid() == id)
        {
          e->Delete();
        }
      }
    }
  }

  World::~World()
  {
    for (auto& el : ents_)
    {
      for (auto& e : el)
      {
        e->Delete();
      }
    }
    UpdateDelete();
  }

  pvector<Entity*> World::GetEntities(TID typeID)
  {
    return ents_[typeID];
  }

  pvector<pvector<Entity*>>& World::GetEntities()
  { 
    return ents_;
  }

  Application* World::application()
  { 
    return application_; 
  }

  PhysicsWorld* World::physics_world()
  {
    return phys_world_; 
  }

  void World::UpdateDelete()
  {
    for (auto& el : ents_)
    {
      for (int i = 0; i < el.size(); ++i)
      {
        if (el[i]->deleted())
        {
          pdelete<Entity>(el[i]);
          el.erase(el.begin() + i);
          --i;
        }
      }
    }
  }

  Entity * World::GetEntity(TID typeID, int i)
  {
    if (i < ents_[typeID].size())
    {
      return ents_[typeID][i];
    }

    return nullptr;
  }

  Entity* World::GetEntity(EID entityID)
  {
    for (auto& el : ents_) 
    {
      for (auto& e : el)
      {
        if (e->eid() == entityID)
        {
          return e;
        }
      }
    }

    return nullptr;
  }
}