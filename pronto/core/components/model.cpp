#include "model.h"
#include "platform/resource.h"
#include "core/entities/empty.h"
#include "core/world.h"
#include "core/application.h"
#include "platform/renderer.h"
#include "mesh.h"
#include "material.h"
#include "resource/pc/mesh_resource.h"
#include "resource/pc/material_resource.h"
#include "resource/pc/model_resource.h" 

#include <assert.h>

namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  Model::Model(Entity* ent) :
    Component(ent),
    model_resource_(nullptr),
    path_("")
  {
  }

  Model::~Model()
  {
    auto renderer = world()->application()->renderer();
    //renderer->UnregisterModel(path_, entity_->transform());
  }

  //-----------------------------------------------------------------------------------------------
  void Model::Update()
  {
  }

  //-----------------------------------------------------------------------------------------------
  void Model::SetModel(const std::string& path)
  {
    auto renderer = world()->application()->renderer();

    //if (path_ != "" && path_ != path)
    //{
    //  unload old model;
    //  renderer->UnregisterModel(path_, entity_->transform());
    //}

    path_ = path;
    model_resource_ = Resource::Get()->LoadModel(path_);

    auto& tree = model_resource_->tree();
    SpawnChildren(transform(), tree);

    renderer->RegisterModel(this);
  }

  //-----------------------------------------------------------------------------------------------
  ModelResource* Model::model_resource()
  {
    return model_resource_;
  }

  //-----------------------------------------------------------------------------------------------
  void Model::SpawnChildren(Transform* parent, Ntree<Primitive>& tree)
  {
    auto& data = tree.data();

    auto ent = world()->CreateEntity<Empty>();
    ent->SetName(model_resource_->path());

    ent->transform()->parent(parent);
    parent->AddChild(ent->transform());

    ent->transform()->ModelMatrix(data.matrix_);

    if (data.mesh_ != nullptr)
    {
      ent->AddComponent<Mesh>()->mesh_resource(data.mesh_);

      if (data.material_ != nullptr)
      {
        ent->AddComponent<Material>()->material_resource(data.material_);
      }
      else
      {
        // Set default material
        ent->AddComponent<Material>()->material_resource(Resource::Get()->LoadMaterial("default"));
        printf("setting default material");
      }
    }

    for (auto& child : tree.children())
    {
      SpawnChildren(ent->transform(), child);
    }
  }
}
