#pragma once
#include "component.h"
#include "type_id.h"
#include "platform/platform.h" 
#include "components/transform.h"

#include <string>
#include <codecvt>

namespace pronto
{
  class World;

  class Entity : public EntID
  {
  public:
    Entity(World* world);

    template<typename T, typename ... Args>
    T* AddComponent(Args... args);

    template<typename T>
    T* GetComponent();

    template<typename T>
    pvector<T*> GetComponents();

    const pvector<pvector<Component*>>* GetAllComponents();

    template<typename T>
    void RemoveComponent();

    template<typename T>
    void RemoveComponents();

    virtual void Update();

    void Delete();

    bool deleted();

    virtual ~Entity();;

    World* world();

    Transform* transform();

    //const std::wstring& w_name();

    const std::string& name();

    void SetName(const std::string& name);

    //void SetName(const std::wstring& w_name);

  protected:
    pvector<pvector<Component*>> components_;
    World* world_;
    bool deleted_ = false;
    Transform* transform_;
    //std::wstring w_name_;
    std::string name_;
  };

  template<typename T, typename ... Args>
  inline T* Entity::AddComponent(Args... args)
  {
    T* c = pnew<T>(this, args...);
    components_[T::tid()].push_back(c);
    return c;
  }

  template<typename T>
  inline T* Entity::GetComponent()
  {
    return components_[T::tid()].size() > 0 ? reinterpret_cast<T*>(components_[T::tid()][0]) : nullptr;
  }

  template<typename T>
  inline pvector<T*> Entity::GetComponents()
  {
    // Makes a new vector that has pointers
    pvector<T*> cc;
    for (auto& c : components_[T::GetTypeID()])
      cc.push_back(c);

    return cc;
  }

  template<typename T>
  inline void Entity::RemoveComponent()
  {
    if (components_[T::tid()].size() > 1)
    {
      std::swap(components_[T::tid()].begin(), components_[T::tid()].rbegin());
    }
    components_[T::tid()].pop_back();
  }

  template<typename T>
  inline void Entity::RemoveComponents()
  {
    components_[T::tid()].clear();
  }
}
