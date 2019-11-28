#pragma once
#include "core/component.h"
#include "core/type_id.h"
#include "resource/pc/material_resource.h"
#include <string>

namespace pronto
{
  class Material : public Component, public CompTID<Material>
  {
  public:
    Material(Entity* ent);
    void material_resource(MaterialResource* material_resource);
    MaterialResource* material_resource();
  private:
    MaterialResource* material_resource_;
  };
}

