#pragma once
#include "core/entity.h"

namespace pronto
{
  class World;
  class Model;

  class Truck : public Entity, public EntTID<Truck>
  {
  public:
    Truck(World* world);
    virtual void Update() override;
  private:
    Model* model_;
  };
}
