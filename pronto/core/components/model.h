#pragma once
#include "core/component.h"
#include "core/type_id.h"

#include <string>

namespace pronto
{
  class ModelResource;

  template<typename T>
  class Ntree;

  struct Primitive;

  class Model : public Component, public CompTID<Model>
  {
  public:
    Model(Entity* ent);
    ~Model();

    void Update() override;

    void SetModel(const std::string& path);

    ModelResource* model_resource();
  private:
    ModelResource* model_resource_; //!< Local cache
    std::string path_;

    void SpawnChildren(Transform* parent, Ntree<Primitive>& tree);
  };
}
