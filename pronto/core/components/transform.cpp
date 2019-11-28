#include "transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  Transform::Transform(Entity* ent) :
    Component(ent),
    parent_(nullptr),
    position_(glm::vec3(0)),
    rotation_(glm::quat(1,0,0,0)),
    scale_(glm::vec3(1)),
    position_matrix_(glm::mat4()),
    rotation_matrix_(glm::mat4()),
    scale_matrix_(glm::mat4()),
    is_dirty_local_(true),
    is_dirty_world_(true)
  {
  }

  //-----------------------------------------------------------------------------------------------
  Transform::Transform(Entity* ent, Transform* parent) :
    Component(ent),
    parent_(parent),
    position_(glm::vec3(0)),
    rotation_(glm::quat(1, 0, 0, 0)),
    scale_(glm::vec3(1)),
    position_matrix_(glm::mat4()),
    rotation_matrix_(glm::mat4()),
    scale_matrix_(glm::mat4()),
    is_dirty_local_(true),
    is_dirty_world_(true)
  {
  }

  void Transform::Update() 
  {
    if (is_dirty_local_ == true)
    {
      // Right here all children are already dirty.
      // But do not recalculate matrices unless asked (ModelMatrix())
      SpankChildren();
    }
  }

  //-----------------------------------------------------------------------------------------------
  glm::vec3 Transform::Forward()
  {
    return rotation_ * glm::vec3(0.f, 0.f, 1.f);
  }

  //-----------------------------------------------------------------------------------------------
  glm::vec3 Transform::Up()
  {
    return rotation_ * glm::vec3(0.f, 1.f, 0.f);
  }

  //-----------------------------------------------------------------------------------------------
  glm::vec3 Transform::Right()
  {
    return rotation_ * glm::vec3(1.f, 0.f, 0.f);
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::TranslateBy(const glm::vec3& vec, const Space& space)
  {
    is_dirty_local_ = true;

    if (space == Space::kLocal)
    {
      position_ += rotation_ * vec;
    }
    else
    {
      position_ += vec;
    }
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::RotateBy(const glm::vec3& axis, const float angle, const Space space)
  {
    if (space == Space::kLocal)
    {
      rotation_ *= glm::angleAxis(angle, axis);
    }
    else
    {
      rotation_ = glm::angleAxis(angle, axis) * rotation_;
    }
  }

  //-----------------------------------------------------------------------------------------------
  Transform* Transform::SetPosition(const glm::vec3& translation)
  {
    position_ = translation;
    is_dirty_local_ = true;
    return this;
  }

  //-----------------------------------------------------------------------------------------------
  Transform* Transform::SetRotation(const glm::quat& rotation)
  {
    rotation_ = rotation;
    is_dirty_local_ = true;
    return this;
  }

  //-----------------------------------------------------------------------------------------------
  Transform* Transform::SetScale(const glm::vec3& scale)
  {
    scale_ = scale;
    is_dirty_local_ = true;
    return this;
  }

  //-----------------------------------------------------------------------------------------------
  const glm::vec3& Transform::position() const
  {
    return position_;
  }

  //-----------------------------------------------------------------------------------------------
  const glm::quat& Transform::rotation() const
  {
    return rotation_;
  }

  //-----------------------------------------------------------------------------------------------
  const glm::vec3& Transform::scale() const
  {
    return scale_;
  }
  
  //-----------------------------------------------------------------------------------------------
  const bool Transform::is_dirty_local() const
  {
    return is_dirty_local_;
  }

  //-----------------------------------------------------------------------------------------------
  const bool Transform::is_dirty_world() const
  {
    return is_dirty_world_;
  }

  //-----------------------------------------------------------------------------------------------
  const glm::mat4& Transform::ModelMatrix(Space space)
  {
    if (is_dirty_local_ == true)
    {
      position_matrix_ = glm::translate(glm::mat4(1.0f), position_);
      rotation_matrix_ = glm::mat4_cast(rotation_);
      scale_matrix_ = glm::scale(scale_);
      model_matrix_ = position_matrix_ * rotation_matrix_ * scale_matrix_;

      is_dirty_world_ = true;
      is_dirty_local_ = false;

      // Right here all children are already dirty.
      // Spank children
      SpankChildren();
    }

    if (is_dirty_world_ == true)
    {
      // calculate world position based on parent world position 
      // TODO: Be sure that all the parents are clean (not dirty)
      if (parent_ != nullptr)
      {
        model_matrix_world_ = parent_->ModelMatrix(Space::kWorld) * model_matrix_;
      }
      else
      {
        model_matrix_world_ = model_matrix_;
      }

      is_dirty_world_ = false;
    }

    return space == Space::kLocal ? model_matrix_ : model_matrix_world_;
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::ModelMatrix(const glm::mat4& mat)
  {
    is_dirty_local_ = true;

    position_ = mat[3].xyz();
    scale_ = glm::vec3(
      glm::length(mat[0].xyz()),
      glm::length(mat[1].xyz()),
      glm::length(mat[2].xyz())
      );

    glm::mat4 rotation = glm::mat4(
      glm::vec4(mat[0].xyz() / scale_.x, 0.f),
      glm::vec4(mat[1].xyz() / scale_.y, 0.f),
      glm::vec4(mat[2].xyz() / scale_.z, 0.f),
      glm::vec4(0, 0, 0, 1)
    );

    rotation_ = glm::quat_cast(rotation);

    //auto& blyet = ModelMatrix(Space::kLocal);
    //assert(blyet == mat);
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::parent(Transform* parent)
  {
    parent_ = parent;
  }

  //-----------------------------------------------------------------------------------------------
  Transform* Transform::parent()
  {
    return parent_;
  }

  //-----------------------------------------------------------------------------------------------
  const std::vector<Transform*>& Transform::children()
  {
    return children_;
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::children(const std::vector<Transform*>& children)
  {
    children_ = children;
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::AddChild(Transform* child)
  {
    children_.push_back(child);
  }

  //-----------------------------------------------------------------------------------------------
  void Transform::AddChildren(const std::vector<Transform*>& children)
  {
    children_.insert(children_.end(), children.begin(), children.end());
  }

  std::vector<Transform*> Transform::GetChildren(bool traverse)
  {
    if (traverse == false)
    {
      return children_;
    }
    else
    {
      std::vector<Transform*> all_children = std::vector<Transform*>();
      TraverseForChildren(this, all_children);
      return all_children;
    }
  }

  void Transform::SpankChildren()
  {
    is_dirty_world_ = true;

    for (auto* child : children_)
    {
      child->SpankChildren();
    }
  }

  void Transform::TraverseForChildren(Transform* child, std::vector<Transform*>& children)
  {
    children.insert(children.begin(), child->children_.begin(), child->children_.end());

    for (auto& childschild : child->children_)
    {
      TraverseForChildren(childschild, children);
    }
  }
}
