#pragma once
#define GLM_FORCE_SWIZZLE
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include "core/component.h"

namespace pronto
{
  enum class Space
  {
    kLocal,
    kWorld
  };

  class Transform : public Component, public CompTID<Transform>
  {
  public:
    Transform(Entity* ent);
    Transform(Entity* ent, Transform* parent);

    void Update() override;

    glm::vec3 Forward();
    glm::vec3 Up();
    glm::vec3 Right();

    void TranslateBy(const glm::vec3& vec, const Space& space);
    void RotateBy(const glm::vec3& axis, const float angle, const Space space = Space::kLocal);

    Transform* SetPosition(const glm::vec3& translation);
    
    template<class T>
    Transform* SetPosition(const T x, const T y, const T z);
    Transform* SetRotation(const glm::quat& rotation);
    Transform* SetScale(const glm::vec3& scale);

    const glm::vec3& position() const;
    const glm::quat& rotation() const;
    const glm::vec3& scale() const ;

    const bool is_dirty_local() const;
    const bool is_dirty_world() const;

    const glm::mat4& ModelMatrix(Space space);
    //TODO: Allow for this to be put in world space
    void ModelMatrix(const glm::mat4& mat);

    void parent(Transform* parent);
    Transform* parent();

    const std::vector<Transform*>& children();
    void children(const std::vector<Transform*>& children);
    void AddChild(Transform* child);
    void AddChildren(const std::vector<Transform*>& children);

    //TODO: Consider caching, but that would mean updating the entire tree when children get altered (set_dirty_children)
    std::vector<Transform*> GetChildren(bool traverse = false);

  private:
    Transform* parent_;
    std::vector<Transform*> children_;

    glm::vec3 position_; //!< The position vector
    glm::quat rotation_; //!< The rotation quat
    glm::vec3 scale_; //!< The scaling vector

    glm::mat4 position_matrix_;
    glm::mat4 rotation_matrix_;
    glm::mat4 scale_matrix_;
    glm::mat4 model_matrix_;
    glm::mat4 model_matrix_world_;

    bool is_dirty_local_;
    bool is_dirty_world_;

    // Makes children dirty
    void SpankChildren(); 
    void TraverseForChildren(Transform* child, std::vector<Transform*>& children);
  };

  template<class T>
  inline Transform* Transform::SetPosition(const T x, const T y, const T z)
  {
    return SetPosition(glm::vec3((float)x, (float)y, (float)z));
  }
}
