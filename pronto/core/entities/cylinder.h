#pragma once
#include "core/entity.h"

namespace pronto
{
  class World;
  class Model;

  class Cylinder : public Entity, public EntTID<Cylinder>
  {
  public:
    Cylinder(World* world);

    virtual void Update() override;
  private:
    Model* model_;
  };
}
