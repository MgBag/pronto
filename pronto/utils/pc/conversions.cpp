#include "conversions.h"
namespace pronto
{
  // Some conversions might not be implemented (yet)
  DXGI_FORMAT ToDXGIFormat(const int type, const int comp_type, bool norm)
  {
    if (type == TINYGLTF_TYPE_SCALAR)
    {
      if (norm) assert(false && "norm not impl");
      switch (comp_type)
      {
      case TINYGLTF_COMPONENT_TYPE_BYTE: // 8
        return DXGI_FORMAT::DXGI_FORMAT_R8_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: // 8
        return DXGI_FORMAT::DXGI_FORMAT_R8_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_SHORT: // 16
        return DXGI_FORMAT::DXGI_FORMAT_R16_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: // 16
        return DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_INT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_FLOAT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
        break;
      case TINYGLTF_COMPONENT_TYPE_DOUBLE: // 64
                                           //return DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT; // ????!?!?!?!?
        assert(false);
        break;
      default:
        break;
      }
    }
    else if (type == TINYGLTF_TYPE_VEC2)
    {
      if (norm) assert(false && "norm not impl");
      switch (comp_type)
      {
      case TINYGLTF_COMPONENT_TYPE_BYTE: // 8
        return DXGI_FORMAT::DXGI_FORMAT_R8G8_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: // 8
        return DXGI_FORMAT::DXGI_FORMAT_R8G8_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_SHORT: // 16
        return DXGI_FORMAT::DXGI_FORMAT_R16G16_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: // 16
        return DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_INT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32G32_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_FLOAT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
        break;
      case TINYGLTF_COMPONENT_TYPE_DOUBLE: // 64
        assert(false);
        break;
      default:
        break;
      }
    }
    else if (type == TINYGLTF_TYPE_VEC3)
    {
      if (norm) assert(false && "norm not impl");
      switch (comp_type)
      {
      case TINYGLTF_COMPONENT_TYPE_BYTE: // 8
        assert(false);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: // 8
        assert(false);
        break;
      case TINYGLTF_COMPONENT_TYPE_SHORT: // 16
        assert(false);
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: // 16
        assert(false);
        break;
      case TINYGLTF_COMPONENT_TYPE_INT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_FLOAT: // 32
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
        break;
      case TINYGLTF_COMPONENT_TYPE_DOUBLE: // 64
        assert(false);
        break;
      default:
        break;
      }
    }
    else if (type == TINYGLTF_TYPE_VEC4)
    {
      switch (comp_type)
      {
      case TINYGLTF_COMPONENT_TYPE_BYTE: // 8
        return norm ? DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SNORM: DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: // 8
        return norm ? DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_SHORT: // 16
        return norm ? DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SNORM : DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: // 16
        return norm ? DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UNORM : DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_INT: // 32
        //assert(false && "type not supported by dx12");
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_SINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: // 32
        //assert(false && "type not supported by dx12");
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT;
        break;
      case TINYGLTF_COMPONENT_TYPE_FLOAT: // 32
        //assert(false && "type not supported by dx12");
        return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
        break;
      case TINYGLTF_COMPONENT_TYPE_DOUBLE: // 64
        assert(false && "type not supported by dx12");
        break;
      default:
        assert(false && "unknown component type encountered!");
        break;
      }
    }
    else if (type == TINYGLTF_TYPE_MAT4)
    {
      if (norm) assert(false && "norm not impl");
      assert(false);
    }

    return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
  }
  D3D_PRIMITIVE_TOPOLOGY toPrimitiveTopology(const int mode)
  {
    switch (mode)
    {
    case 0: //TINYGLTF_MODE_POINTS
      return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
      break;
    case 1: //TINYGLTF_MODE_LINE
      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
      break;
    case 2: //TINYGLTF_MODE_LINE_LOOP
      assert(false);
      return D3D_PRIMITIVE_TOPOLOGY_LINELIST; // Doesn't it always do that?
      break;
    case 3: //TINYGLTF_MODE_LINE_STRIP
      return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
      break;
    case 4: //TINYGLTF_MODE_TRIANGLES
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
      break;
    case 5: //TINYGLTF_MODE_TRIANGLE_STRIP
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
      break;
    case 6: //TINYGLTF_MODE_TRIANGLE_FAN
      assert(false);
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
      break;
    default:
      assert(false);
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
      break;
    }
  }

  glm::vec3 BToGlm(const btVector3& v)
  {
    return glm::vec3(v.x(), v.y(), v.z());
  }

  glm::vec4 BToGlm(const btVector4& v)
  {
    return glm::vec4(v.x(), v.y(), v.z(), v.w());
  }

  glm::mat3x3 BToGlm(const btMatrix3x3& mat)
  {
    // Bullet matrix to GLM matrix needs to be transposed, done here by using getColumn
    return glm::mat3x3(
      BToGlm(mat.getColumn(0)),
      BToGlm(mat.getColumn(1)),
      BToGlm(mat.getColumn(2)));
  }

  glm::quat BToGlm(const btQuaternion& quat)
  {
    return glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
  }

  glm::mat4x4 BToGlm(const btTransform& transform)
  {
    glm::mat4x4 glm_matrix = glm::mat4x4();
    glm::mat3x3 rotation = BToGlm(transform.getBasis());
    glm_matrix[0] = { rotation[0], 0.0f };
    glm_matrix[1] = { rotation[1], 0.0f };
    glm_matrix[2] = { rotation[2], 0.0f };
    glm_matrix[3] = { BToGlm(transform.getOrigin()), 1.0f };

    return glm_matrix;
  }

  btVector3 GlmToB(const glm::vec3& v)
  {
    return btVector3(v.x, v.y, v.z);
  }

  btVector4 GlmToB(const glm::vec4& v)
  {
    return btVector4(v.x, v.y, v.z, v.w);
  }

  btMatrix3x3 GlmToB(const glm::mat3x3& mat)
  {
    // GLM matrix to Bullet matrix needs to be transposed, which is done directly here
    return btMatrix3x3(
      mat[0].x, mat[1].x, mat[2].x,
      mat[0].y, mat[1].y, mat[2].y,
      mat[0].z, mat[1].z, mat[2].z);
  }

  btQuaternion GlmToB(const glm::quat& quat)
  {
    return btQuaternion(quat.x, quat.y, quat.z, quat.w);
  }

  btTransform GlmToB(const glm::mat4x4& transform)
  {
    btMatrix3x3 basis = btMatrix3x3();

    // GLM matrix to Bullet matrix needs to be transposed, which is done directly here
    basis[0] = GlmToB({ transform[0].x, transform[1].x, transform[2].x });
    basis[1] = GlmToB({ transform[0].y, transform[1].y, transform[2].y });
    basis[2] = GlmToB({ transform[0].z, transform[1].z, transform[2].z });

    btVector3 origin = GlmToB({ transform[3].x, transform[3].y, transform[3].z });

    return btTransform(basis, origin);
  }

  LPCSTR ShaderTypeToCompileTarget(D3D12_SHADER_VERSION_TYPE type)
  {
    switch (type)
    {
    case D3D12_SHVER_COMPUTE_SHADER:
      return "cs_5_1";	//DirectCompute 5.0 (compute shader)
    case D3D12_SHVER_DOMAIN_SHADER:
      return "ds_5_1";	//Domain shader
    case D3D12_SHVER_GEOMETRY_SHADER:
      return "gs_5_1";	//Geometry shader
    case D3D12_SHVER_HULL_SHADER:
      return "hs_5_1";	//Hull shader
    case D3D12_SHVER_PIXEL_SHADER:
      return "ps_5_1";	//Pixel shader
    case D3D12_SHVER_VERTEX_SHADER:
      return "vs_5_1";	//Vertex shader
    default:
      break;
    }

    return "";
  }
}
