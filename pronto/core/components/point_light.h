#pragma once
#include "core/component.h"
#include "core/type_id.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace pronto
{
  struct PointLightBuffer
  {
    PointLightBuffer(
      const glm::vec4& world_pos,
      const glm::vec4& view_pos,
      const glm::vec4& color,
      const float attenuation,
      const float intensity);

    glm::vec4 world_pos_;
    glm::vec4 view_pos_;
    glm::vec4 color_;
    float attenuation_;
    float intensity_;
    glm::vec2 padding_;
  };

  class PointLight : public Component, public CompTID<PointLight>
  {
  public:
    PointLight(Entity* ent);
    ~PointLight();

    void Update() override;

    void set_color(const glm::vec3& color);
    void set_attenuation(const float attenuation);
    void set_intensity(const float intensity);

    const glm::vec3& color();
    float attenuation();
    float intensity();

    // Please do ignore this horrible solution.
    PointLightBuffer CreateStruct();

  private:
    glm::vec3 color_;
    float attenuation_;
    float intensity_;
  };
}
