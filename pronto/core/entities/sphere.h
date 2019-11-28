#pragma once
#include "core/entity.h"

namespace pronto
{
  class World;
  class Model;

  class Sphere : public Entity, public EntTID<Sphere>
  {
  public:
    Sphere(World* world);
    virtual void Update() override;
  private:
    Model* model_;
  };
}
