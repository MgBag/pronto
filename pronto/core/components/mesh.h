#pragma once
#include "core/component.h"
#include "core/type_id.h"
#include <string>

namespace pronto
{
  class Model;
  class MeshResource;

  class Mesh : public Component, public CompTID<Mesh>
  {
  public:
    Mesh(Entity* ent);

    void Update() override;

    void mesh_resource(MeshResource* mesh);

    MeshResource* mesh_resource();
  private:
    MeshResource* mesh_resource_; //!< Local cache
  };
}
