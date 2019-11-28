#pragma once
#include "core/entity.h"

namespace pronto
{
  class World;
  class Model;

  class Empty : public Entity, public EntTID<Empty>
  {
  public:
    Empty(World* world);
  };
}
