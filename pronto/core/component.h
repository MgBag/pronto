#pragma once
#include "type_id.h"

namespace pronto
{
  class Entity;
  class Transform;
  class World;

  class Component
  {
    friend class Entity;
  public:

    virtual ~Component() {};
    Entity* entity() const;
    Transform* transform();
    World* world();

  protected:
    template<typename ... Args>
    Component(Entity* entity, Args... args) :
      entity_(entity), deleted_(false), world_(nullptr), transform_(nullptr)
    {}

    virtual void Update() {};

    Entity* entity_;
    
    bool deleted_;

  private:
    Transform* transform_;
    World* world_;
  };

}
