#pragma once
#include "core/component.h"
#include "core/type_id.h"
#include <glm/mat4x4.hpp>

namespace pronto
{
  class Entity;

  class Camera : public Component, public CompTID<Camera>
  {
  public:
    Camera(Entity* ent);
    ~Camera();

    virtual void Update() override {}

    void fov(const float fov);
    float fov() const;

    void near_plane(const float near_plane);
    float near_plane() const;

    void far_plane(const float far_plane);
    float far_plane() const;

    void width(const unsigned int width);
    unsigned int width() const;

    void height(const unsigned int height);
    unsigned int height() const;

    const glm::mat4& GetViewMatrix();
    const glm::mat4& GetProjectionMatrix();
    
  private:
    float hash_dirty_;
    float fov_;
    unsigned int width_;
    unsigned int height_;
    float near_plane_;
    float far_plane_;
    glm::mat4 view_matrix_;
    glm::mat4 projection_matrix_;
  };
}
