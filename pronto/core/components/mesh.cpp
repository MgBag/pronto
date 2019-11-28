#include "mesh.h"
#include "platform/resource.h"
#include "core/entity.h"
#include "core/world.h"
#include "core/application.h"
#include "platform/renderer.h"
#include "resource/pc/mesh_resource.h"

namespace pronto
{
  Mesh::Mesh(Entity* ent) :
    Component(ent),
    mesh_resource_(nullptr)
  {
  }

  void Mesh::Update()
  {
  }

  void Mesh::mesh_resource(MeshResource* mesh)
  {
    mesh_resource_ = mesh;
  }
  
  MeshResource* Mesh::mesh_resource()
  {
    return mesh_resource_;
  }
}
