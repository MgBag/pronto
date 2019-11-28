#pragma once
#include "entity.h"
#include "physics/physics_world.h"

namespace pronto
{
  class Application;

  class World
  {
  public:
    World(Application* app);
    ~World();

    void Update();

    template<typename T, typename ... Args>
    T* CreateEntity(Args... args);
    template<typename T>
    void DeleteEntity(int i = -1);

    void DeleteEntity(TID id, int i = -1);
    void DeleteEntity(EID id);

    template<typename T>
    T* GetEntity(int i = 0);

    Entity* GetEntity(TID typeID, int i = 0);
    Entity* GetEntity(EID entityID);

    pvector<Entity*> GetEntities(TID typeID);
    template<typename T>
    pvector<T*> GetEntities();

    pvector<pvector<Entity*>>& GetEntities();
    Application* application();
    PhysicsWorld* physics_world();

  private:
    pvector<pvector<Entity*>> ents_;
    Application* application_; //!< Pointer to parent
    PhysicsWorld* phys_world_;

    void UpdateDelete();
  };

  template<typename T, typename ... Args>
  inline T* World::CreateEntity(Args... args)
  {
    T* e = pnew<T>(this, args...);
    ents_[T::tid()].push_back(e);
    return e;
  }

  template<typename T>
  inline T * World::GetEntity(int i)
  {
    if (ents_[T::tid()].size() >= i + 1)
      return static_cast<T*>(ents_[T::tid()][i]);
    return nullptr;
  }

  template<typename T>
  pvector<T*> World::GetEntities()
  {
    return ents_[T::tid()];
  }

  template<typename T>
  inline void World::DeleteEntity(int i)
  {
    if (i == -1)
    {
      ents_[T::tid()].pop_back();
    }
    else if (i < (int)ents_[T::tid()].size())
    {
      ents_[T::tid()].erase(ents_[T::tid()].begin() + i);
    }
  }
}
