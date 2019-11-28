#pragma once
#include <string>
#include <dxgiformat.h>
#include <d3d12.h>

namespace pronto
{
  namespace graphics
  {
    class CommandList;
  }

  // TODO: Split this up into attribute and index buffer?
  struct MeshBuffer
  {
    MeshBuffer() { buffer_ = nullptr; }
    MeshBuffer(
      const unsigned char* buffer,
      size_t stride,
      size_t count,
      DXGI_FORMAT format) :
      buffer_(buffer),
      stride_(stride),
      count_(count),
      format_(format)
    {}

    const unsigned char* buffer_;
    size_t stride_;
    size_t count_;
    DXGI_FORMAT format_;
    D3D12_VERTEX_BUFFER_VIEW vbv_;

    bool is_valid() { return buffer_ != nullptr; }
  };

  struct IndexBuffer
  {
    IndexBuffer() { buffer_ = nullptr; }

    const unsigned char* buffer_;
    size_t stride_;
    size_t count_;
    DXGI_FORMAT format_;
    D3D_PRIMITIVE_TOPOLOGY topology_;
    D3D12_INDEX_BUFFER_VIEW ibv_;

    bool is_valid() { return buffer_ != nullptr; }
  };

  class MeshResource
  {
  public:
    MeshResource() {};

    MeshResource(
      const std::string& path,
      unsigned char* positions,
      size_t positions_stride,
      size_t positions_count,
      DXGI_FORMAT positions_format,
      unsigned char* indices,
      size_t indices_stride,
      size_t indices_count,
      DXGI_FORMAT indices_format
    );

    ~MeshResource();

    const std::string& path() const;
    void path(const std::string& path);

    MeshBuffer& positions();
    IndexBuffer& indices();
    MeshBuffer& normals();
    MeshBuffer& tex_coords();
    MeshBuffer& tex_coords2();
    MeshBuffer& vert_colors();

    void positions(const MeshBuffer& positions);
    void indices(const IndexBuffer& indices);
    void normals(const MeshBuffer& normals);
    void tex_coords(const MeshBuffer& tex_coords);
    void tex_coords2(const MeshBuffer& tex_coords2);
    void vert_colors(const MeshBuffer& vert_colors);

    void Upload(graphics::CommandList* command_list);

  private:
    std::string path_;

    MeshBuffer positions_;
    IndexBuffer indices_;
    MeshBuffer normals_;
    MeshBuffer tex_coords_;
    MeshBuffer tex_coords2_;
    MeshBuffer vert_colors_;

    void UploadBuffer(
      graphics::CommandList* command_list,
      MeshBuffer& buffer);

    void UploadIndexBuffer(
      graphics::CommandList* command_list,
      IndexBuffer& buffer);
  };
}
