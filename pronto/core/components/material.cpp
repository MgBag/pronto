#include "material.h"
#include "platform/resource.h"
#include "core/entity.h"
#include "core/world.h"
#include "core/application.h"
#include "platform/renderer.h"

namespace pronto
{
  Material::Material(Entity* ent) :
    Component(ent)
  {
  }

  void Material::material_resource(MaterialResource* material_resource)
  {
    material_resource_ = material_resource;
  }

  MaterialResource* Material::material_resource()
  {
    return material_resource_;
  }
}
