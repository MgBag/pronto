#include "camera.h"
#include "core/entity.h"
#include <DirectXMath.h>

namespace pronto
{
  Camera::Camera(Entity* ent) :
    Component(ent),
    hash_dirty_(0.f),
    fov_(90.f),
    width_(10),
    height_(10),
    near_plane_(0.1f),
    far_plane_(1000.f),
    view_matrix_(glm::mat4()),
    projection_matrix_(glm::mat4())
  {
  }

  Camera::~Camera()
  {
  }

  void Camera::fov(const float fov)
  {
    fov_ = fov;
  }

  float Camera::fov() const
  {
    return fov_;
  }

  void Camera::near_plane(const float near_plane)
  {
    near_plane_ = near_plane;
  }

  float Camera::near_plane() const
  {
    return near_plane_;
  }

  void Camera::far_plane(const float far_plane)
  {
    far_plane_ = far_plane;
  }

  float Camera::far_plane() const
  {
    return far_plane_;
  }

  void Camera::width(const unsigned int width)
  {
    width_ = width;
  }

  unsigned int Camera::width() const
  {
    return width_;
  }

  void Camera::height(const unsigned int height)
  {
    height_ = height;
  }

  unsigned int Camera::height() const
  {
    return height_;
  }

  const glm::mat4& Camera::GetViewMatrix()
  {
    //entity()->transform()->ModelMatrix(Space::kWorld); // Make the transform clean once more.

    //glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), -entity()->transform()->position());
    //// Since we know the rotation matrix is orthonormalized, we can simply 
    //// transpose the rotation matrix instead of inversing. ?!?!?!?!
    //glm::mat4 rotate = glm::transpose(glm::mat4_cast(entity()->transform()->rotation()));

    
    auto translation = glm::translate(glm::mat4(1.0f), -transform()->position());

    auto rotation = glm::mat4_cast(transform()->rotation());
    rotation = glm::transpose(rotation);

    view_matrix_ = rotation * translation;

    //view_matrix_ = 
    //  glm::lookAtLH(transform()->position(), transform()->Forward(), transform()->Up());

    return view_matrix_;
  }

  const glm::mat4& Camera::GetProjectionMatrix()
  {
    // Simple has, assumes only one value changes at a time
    if (fov_ + near_plane_ + far_plane_ + width_ + height_ != hash_dirty_)
    {
      projection_matrix_ = glm::perspectiveFovLH_ZO(
        glm::radians(fov_),
        (float)width_,
        (float)height_,
        near_plane_,
        far_plane_
      );

      hash_dirty_ = fov_ + near_plane_ + far_plane_ + width_ + height_;
    }

    return projection_matrix_;
  }
}