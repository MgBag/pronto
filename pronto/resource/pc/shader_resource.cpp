#include "shader_resource.h"
#include <assert.h>
#include <fstream>
#include <d3dcompiler.h>
#include <utils/pc/conversions.h>

namespace pronto
{
  ShaderResource::ShaderResource(
    const std::string& path,
    D3D12_SHADER_VERSION_TYPE type) :
    path_(path),
    type_(type)
  {
    assert(std::filesystem::exists(path));
    last_modified_ = std::filesystem::last_write_time(path);
    UpdateShader();
  }

  bool ShaderResource::FileModified()
  {
    //TODO: Make this function on a callback system
    //https://www.codeproject.com/Articles/4692/How-to-get-a-notification-if-change-occurs-in-a-sp
    return std::filesystem::last_write_time(path_) > last_modified_;
  }

  const std::string& ShaderResource::path()
  {
    return path_;
  }

  const ID3DBlob* ShaderResource::UpdateShader()
  {
    buffer_.clear();
    std::ifstream file(path_, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    
    file.seekg(0, std::ios::beg);

    buffer_.resize(size);
    assert(file.read(buffer_.data(), size) && "failed to load data");
    last_modified_ = std::filesystem::last_write_time(path_);

    CompileShader();

    return shader_;
  }

  const std::vector<char>& ShaderResource::buffer()
  {
    return buffer_;
  }

  ID3DBlob* ShaderResource::shader()
  {
    return shader_;
  }

  const D3D12_SHADER_VERSION_TYPE ShaderResource::type()
  {
    return type_;
  }

  void ShaderResource::CompileShader()
  {
    //TODO: Clean up blob before reuse
    ID3DBlob* error_blob = nullptr;
    HRESULT hr = D3DCompile2(
      buffer_.data(),
      buffer_.size(),
      path_.c_str(),
      nullptr,
      nullptr,
      "main",
      ShaderTypeToCompileTarget(type_),
      D3DCOMPILE_SKIP_OPTIMIZATION, 0, 0, nullptr, 0, &shader_, &error_blob);

    if (FAILED(hr) && error_blob != nullptr)
    {
      char* out = (char*)error_blob->GetBufferPointer();
      printf(out);
      DWORD error = ::GetLastError();
      assert(false);
    }
  }
}
