#pragma once
#include "core/component.h"
#include "core/type_id.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace pronto
{
  struct DirectionalLightBuffer
  {
    DirectionalLightBuffer(
      const glm::vec3& direction,
      const float intensity);

    glm::vec3 direction_;
    float intensity_;
  };

  // TODO NAME THIS SPOT LIGHT
  class DirectionalLight : public Component, public CompTID<DirectionalLight>
  {
  public:
    DirectionalLight(Entity* ent);
    ~DirectionalLight();

    void Update() override;

    void set_intensity(const float intensity);

    float intensity();

    // Please do ignore this horrible solution.
    DirectionalLightBuffer CreateStruct();

  private:
    float intensity_;
  };
}
