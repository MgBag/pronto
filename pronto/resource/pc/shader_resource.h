#pragma once
#include <string>
#include <filesystem>
#include <chrono>
#include <d3dcommon.h>
#include <d3d12shader.h>

namespace pronto
{
  class ShaderResource
  {
  public:
    ShaderResource(
      const std::string& path,
      D3D12_SHADER_VERSION_TYPE type);

    bool FileModified();
    const std::string& path();
    const ID3DBlob* UpdateShader();
    const std::vector<char>& buffer();
    ID3DBlob* shader();
    const D3D12_SHADER_VERSION_TYPE type();

  private:
    void CompileShader();

    std::string path_;
    std::filesystem::file_time_type last_modified_;
    std::vector<char> buffer_;

    ID3DBlob* shader_;
    D3D12_SHADER_VERSION_TYPE type_;
  };
}
