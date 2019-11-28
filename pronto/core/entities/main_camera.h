#pragma once
#include "core/entity.h"
#include "core/type_id.h"
#include "core/components/camera.h"

namespace pronto
{
  class World;

  class MainCamera : public Entity, public EntTID<MainCamera>
  {
  public:
    MainCamera(World* world);
    ~MainCamera() {};

    void Update() override;

    Camera* cam() const;

  private:
    Camera* cam_;
  };
}
