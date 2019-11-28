#pragma once
#include "core/entity.h"

namespace pronto
{
  class World;
  class Model;

  class Box : public Entity, public EntTID<Box>
  {
  public:
    Box(World* world);
    virtual void Update() override;
  private:
    Model* model_;
  };
}
