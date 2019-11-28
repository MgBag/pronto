#pragma once
#include <glm/mat4x4.hpp>

namespace pronto
{
  namespace graphics
  {
    struct ModelViewProjection
    {
      glm::mat4 model_matrix_;
      glm::mat4 view_matrix_;
      glm::mat4 projection_matrix_;
    };
  }
}