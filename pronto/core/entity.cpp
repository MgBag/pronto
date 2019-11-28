#include "entity.h"

namespace pronto
{

  Entity::Entity(World* world) :
    name_("Entity")

  {
    world_ = world;
    components_ = pvector<pvector<Component*>>(CompTIDBase::size());
    transform_ = AddComponent<Transform>();
  }
  const pvector<pvector<Component*>>* Entity::GetAllComponents()
  {
    return &components_;
  }

  void Entity::Update()
  {
    for (auto& t : components_)
    {
      for (auto& c : t)
      {
        c->Update();
      }
    }
  }

  void Entity::Delete()
  {
    deleted_ = true;
  }

  bool Entity::deleted()
  {
    return deleted_;
  }

  Entity::~Entity()
  {
    for (auto& t : components_)
    {
      for (auto& c : t)
      {
        pdelete<Component>(c);
      }

      t.clear();
    }

    components_.clear();
  }

  World* Entity::world()
  {
    return world_;
  }

  Transform* Entity::transform()
  {
    return transform_;
  }

  //const std::wstring& Entity::w_name()
  //{
  //  return w_name_;
  //}

  const std::string& Entity::name()
  {
    return name_;
  }

  void Entity::SetName(const std::string& name)
  {
    name_ = name;
    //w_name_ = std::wstring(name.begin(), name.end());
  }

  //void Entity::SetName(const std::wstring& w_name)
  //{
  //  w_name_ = w_name;

  //  //setup converter
  //  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
  //  name_ = converter.to_bytes(w_name);
  //}
}
