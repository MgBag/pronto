#include "point_light.h"
#include "platform/resource.h"
#include "core/entity.h"
#include "core/world.h"
#include "core/application.h"
#include "platform/renderer.h"

namespace pronto
{
  PointLight::PointLight(Entity* ent) :
    Component(ent),
    attenuation_(100.f),
    color_(glm::vec3(1, 1, 1)),
    intensity_(5000.f)
  {
    entity_->world()->application()->renderer()->RegisterPointLight(this);
  }

  PointLight::~PointLight()
  {
    entity_->world()->application()->renderer()->UnRegisterPointLight(this);
  }
  
  void PointLight::Update()
  {
  }

  void PointLight::set_attenuation(const float attenuation)
  {
    attenuation_ = attenuation;
  }

  void PointLight::set_color(const glm::vec3& color)
  {
    color_ = color;
  }

  void PointLight::set_intensity(const float intensity)
  {
    intensity_ = intensity;
  }

  float PointLight::attenuation()
  {
    return attenuation_;
  }

  const glm::vec3& PointLight::color()
  {
    return color_;
  }

  float PointLight::intensity()
  {
    return intensity_;
  }

  PointLightBuffer PointLight::CreateStruct()
  {
    return PointLightBuffer(
      glm::vec4(entity_->transform()->position(), 1),
      glm::vec4(1, 1, 1, 1),
      glm::vec4(color_, 1),
      attenuation_,
      intensity_);
  }
  
  PointLightBuffer::PointLightBuffer(
    const glm::vec4& world_pos,
    const glm::vec4& view_pos,
    const glm::vec4& color,
    const float attenuation,
    const float intensity) :
    world_pos_(world_pos),
    view_pos_(view_pos),
    color_(color),
    attenuation_(attenuation),
    intensity_(intensity)
  {
  }
}
