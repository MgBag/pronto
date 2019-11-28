#include "component.h"
#include "core/entity.h"

namespace pronto
{
  Entity* Component::entity() const 
  { 
    return entity_;
  }

  Transform* Component::transform()
  {
    if (transform_ == nullptr)
    {
      transform_ = entity_->transform();
    }

    return transform_;
  }

  World* Component::world()
  {
    if (world_ == nullptr)
    {
      world_ = entity_->world();
    }

    return world_;
  }
}
