#pragma once
#include <glm/vec3.hpp>

#define COM_NO_WINDOWS_H
#include "graphics/pc/d3dx12.h"
#undef COM_NO_WINDOWS_H

#include <tiny_gltf.h>

#include <assert.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <LinearMath/btVector3.h>
#include <LinearMath/btMatrixX.h>
#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>

#include <d3d12shader.h>

namespace pronto
{
  DXGI_FORMAT ToDXGIFormat(const int type, const int comp_type, bool norm = false);

  D3D_PRIMITIVE_TOPOLOGY toPrimitiveTopology(const int mode);

  glm::vec3 BToGlm(const btVector3& v);

  glm::vec4 BToGlm(const btVector4& v);

  glm::mat3x3 BToGlm(const btMatrix3x3& mat);

  glm::quat BToGlm(const btQuaternion& quat);

  glm::mat4x4 BToGlm(const btTransform& transform);

  btVector3 GlmToB(const glm::vec3& v);

  btVector4 GlmToB(const glm::vec4& v);

  btMatrix3x3 GlmToB(const glm::mat3x3& mat);

  btQuaternion GlmToB(const glm::quat& quat);

  btTransform GlmToB(const glm::mat4x4& transform);

  LPCSTR ShaderTypeToCompileTarget(D3D12_SHADER_VERSION_TYPE type);

  template <class T>
  glm::vec4 ToVec4(const std::vector<T> vector);

  template<class T>
  glm::vec4 ToVec4(const std::vector<T> vector)
  {
    return glm::vec4(
      (float)vector[0],
      (float)vector[1],
      (float)vector[2],
      (float)vector[3]
      );
  }

  //https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
  //glm::vec4 GetPosition(const glm::mat4& trans)
}