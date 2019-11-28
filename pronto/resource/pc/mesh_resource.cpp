#include "graphics/pc/d3d12_command_list.h"
#include "graphics/pc/d3d12_upload_buffer.h"
#include "graphics/pc/d3d12_constant_buffer_heap.h"
#include "mesh_resource.h"
#include <assert.h>

namespace pronto
{
  MeshResource::MeshResource(
    const std::string& path,
    unsigned char* positions,
    size_t positions_stride,
    size_t positions_count,
    DXGI_FORMAT positions_format,
    unsigned char* indices,
    size_t indices_stride,
    size_t indices_count,
    DXGI_FORMAT indices_format) :
    path_(path)
  {
  }

  MeshResource::~MeshResource()
  {
  }

  const std::string& pronto::MeshResource::path() const
  {
    return path_;
  }

  void MeshResource::path(const std::string& path)
  {
    path_ = path;
  }

 
  MeshBuffer& MeshResource::positions()
  {
    return positions_;
  }

  IndexBuffer& MeshResource::indices()
  {
    return indices_;
  }

  MeshBuffer& MeshResource::normals()
  {
    return normals_;
  }

  MeshBuffer& MeshResource::tex_coords()
  {
    return tex_coords_;
  }

  MeshBuffer& MeshResource::tex_coords2()
  {
    return tex_coords2_;
  }

  MeshBuffer& MeshResource::vert_colors()
  {
    return vert_colors_;
  }

  void MeshResource::positions(const MeshBuffer& positions)
  {
    positions_ = positions;
  }

  void MeshResource::indices(const IndexBuffer& indices)
  {
    indices_ = indices;
  }

  void MeshResource::normals(const MeshBuffer& normals)
  {
    normals_ = normals;
  }

  void MeshResource::tex_coords(const MeshBuffer& tex_coords)
  {
    tex_coords_ = tex_coords;
  }

  void MeshResource::tex_coords2(const MeshBuffer& tex_coords2)
  {
    tex_coords2_ = tex_coords2;
  }

  void MeshResource::vert_colors(const MeshBuffer& vert_colors)
  {
    vert_colors_ = vert_colors;
  }

  void MeshResource::Upload(graphics::CommandList* command_list)
  {
    if (positions_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, positions_);
    }

    if (indices_.buffer_ != nullptr)
    {
      UploadIndexBuffer(command_list, indices_);
    }

    if (normals_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, normals_);
    }

    if (tex_coords_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, tex_coords_);
    }

    if (tex_coords2_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, tex_coords2_);
    }

    if (tex_coords2_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, vert_colors_);
    }
  }

  void MeshResource::UploadBuffer(
    graphics::CommandList* command_list,
    MeshBuffer& buffer)
  {
    assert(buffer.stride_ != -1 && "somethings fucky");
    assert(buffer.stride_ < 2048 && "somethings fucky");

    size_t size = buffer.stride_ * buffer.count_;

    auto allocation = command_list->const_buffer_heap()->Upload(buffer.buffer_, size);
      //command_list->upload_buffer()->Allocate(size, buffer.stride_);

    //memcpy(allocation.CPU, buffer.buffer_, size);

    buffer.vbv_.BufferLocation = allocation;//.GPU;
    buffer.vbv_.SizeInBytes = (UINT)size;
    buffer.vbv_.StrideInBytes = (UINT)buffer.stride_;
  }

  void MeshResource::UploadIndexBuffer(
    graphics::CommandList* command_list,
    IndexBuffer& buffer)
  {
    assert(buffer.stride_ != -1 && "somethings fucky");
    assert(buffer.stride_ < 2048 && "somethings fucky");

    size_t size = buffer.stride_ * buffer.count_;

    auto allocation = command_list->const_buffer_heap()->Upload(buffer.buffer_, size);
    //command_list->upload_buffer()->Allocate(size, buffer.stride_);

    //memcpy(allocation.CPU, buffer.buffer_, size);

    buffer.ibv_.BufferLocation = allocation;//.GPU;
    buffer.ibv_.Format = buffer.format_;
    buffer.ibv_.SizeInBytes = (UINT)size;
  }
}
