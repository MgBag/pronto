#include "directional_light.h"
#include "core/components/transform.h"
#include "core/world.h"
#include "core/application.h"
#include "platform/renderer.h"
#include <glm/geometric.hpp>

namespace pronto
{
  DirectionalLight::DirectionalLight(Entity* ent) :
    Component(ent),
    intensity_(1.f)
  {
    //world()->application()->renderer()->RegisterDirectionalLight(this);
  }

  DirectionalLight::~DirectionalLight()
  {
  }

  void DirectionalLight::Update()
  {
  }

  void DirectionalLight::set_intensity(const float intensity)
  {
    intensity_ = intensity;
  }

  float DirectionalLight::intensity()
  {
    return intensity_;
  }

  DirectionalLightBuffer DirectionalLight::CreateStruct()
  {
    return DirectionalLightBuffer(transform()->Forward(), intensity_);
  }

  DirectionalLightBuffer::DirectionalLightBuffer(
    const glm::vec3& direction, 
    const float intensity) :
    direction_(direction),
    intensity_(intensity)
  {
  }
}