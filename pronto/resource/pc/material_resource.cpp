#include "material_resource.h"
#include "graphics/pc/d3d12_command_list.h"
#include "graphics/pc/d3d12_constant_buffer_heap.h"
#include "graphics/pc/d3dx12.h"
#include <wtypes.h>
#include <dxgiformat.h>
#include <Windows.h>
#include <wincodec.h>
#include <assert.h>
#include <d3d12.h>

namespace pronto
{
  //-----------------------------------------------------------------------------------------------
  MaterialResource::MaterialResource() {}

  //-----------------------------------------------------------------------------------------------
  MaterialResource::MaterialResource(
    const std::string& path,
    unsigned char* texture,
    size_t texture_stride,
    size_t texture_count,
    DXGI_FORMAT texture_format,
    size_t height,
    size_t width) :
    path_(path)
  {
    texture_.buffer_ = texture;
    texture_.stride_ = texture_stride;
    texture_.count_ = texture_count;
    texture_.format_ = texture_format;
    texture_.width_ = width;
    texture_.height_ = height;
  }

  //-----------------------------------------------------------------------------------------------
  MaterialResource::~MaterialResource()
  {
  }

  //-----------------------------------------------------------------------------------------------
  const std::string& MaterialResource::path() const
  {
    return path_;
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::path(const std::string& path)
  {
    path_ = path;
  }

  //-----------------------------------------------------------------------------------------------
  ImageBuffer& MaterialResource::texture()
  {
    return texture_;
  }

  //-----------------------------------------------------------------------------------------------
  ImageBuffer& MaterialResource::normal_map()
  {
    return normal_map_;
  }

  //-----------------------------------------------------------------------------------------------
  BaseColor& MaterialResource::base_color()
  {
    return base_color_;
  }

  graphics::PrimitiveType MaterialResource::type()
  {
    return type_;
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::texture(const ImageBuffer& texture)
  {
    texture_ = texture;
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::normal_map(const ImageBuffer& normal_map)
  {
    normal_map_ = normal_map;
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::base_color(const BaseColor& base_color)
  {
    base_color_ = base_color;
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::base_color(const glm::vec4& color)
  {
    base_color_.color_ = color;
  }

  void MaterialResource::type(graphics::PrimitiveType type)
  {
    type_ = type;
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::Upload(graphics::CommandList* command_list)
  {
    if (texture_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, texture_);
    }

    if (normal_map_.buffer_ != nullptr)
    {
      UploadBuffer(command_list, normal_map_);
    }

    if (base_color_.is_valid_ == true)
    {
      UploadBaseColor(command_list);
    }
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::UploadBuffer(graphics::CommandList* command_list, ImageBuffer& buffer)
  {
    //TODO: Check the scope of these ere values  
    size_t byte_per_row;
    size_t image_size;
    D3D12_RESOURCE_DESC resource_desc; // this

    IWICBitmapFrameDecode* wic_frame = NULL;

    bool image_converted = false;
    IWICFormatConverter* wic_converter = NULL;


    assert(texture_.format_ != DXGI_FORMAT_UNKNOWN);

    byte_per_row = buffer.width_ * buffer.channel_depth_ * buffer.component_ / 8; // number of bytes in each row of the image data
    image_size = byte_per_row * buffer.height_; // total image size in bytes

    // allocate enough memory for the raw image data, and set imageData to point to that memory

    // now describe the texture with the information we have obtained from the image
    resource_desc = {};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Alignment = 0;
    // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB 
    //(4MB for multi-sampled textures)
    resource_desc.Width = buffer.width_;
    // width of the texture
    resource_desc.Height = (UINT)buffer.height_;
    // height of the texture
    resource_desc.DepthOrArraySize = 1;
    // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures
    //(we only have one image, so we set 1)
    resource_desc.MipLevels = 1;
    // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
    resource_desc.Format = buffer.format_;
    // This is the dxgi format of the image (format of the pixels)
    resource_desc.SampleDesc.Count = 1;
    // This is the number of samples per pixel, we just want 1 sample
    resource_desc.SampleDesc.Quality = 0;
    // The quality level of the samples. Higher is better quality, but worse performance
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    // no flags

    command_list->device()->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
      D3D12_HEAP_FLAG_NONE,
      &resource_desc,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&buffer.resource_));

    buffer.resource_->SetName(L"Texture resource");


    UINT64 textureUploadBufferSize;
    //// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    //// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    //// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
    ////textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    command_list->device()->GetCopyableFootprints(
      &resource_desc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    ID3D12Resource* upload;

    // now we create an upload heap to upload our texture to the GPU
    HRESULT hr = command_list->device()->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
      D3D12_HEAP_FLAG_NONE, // no flags
      &CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
      D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
      nullptr,
      IID_PPV_ARGS(&upload));

    upload->SetName(L"Doing it again");

    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false);
    }

    upload->SetName(L"Texture Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA texture_data = {};
    texture_data.pData = buffer.buffer_; // pointer to our image data
    texture_data.RowPitch = byte_per_row; // size of all our triangle vertex data
    texture_data.SlicePitch = image_size; // also the size of our triangle vertex data

    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(
      command_list->command_list(), buffer.resource_, upload, 0, 0, 1, &texture_data);

    //transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)

    command_list->command_list()->ResourceBarrier(
      1,
      &CD3DX12_RESOURCE_BARRIER::Transition(
        buffer.resource_,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
  }

  //-----------------------------------------------------------------------------------------------
  void MaterialResource::UploadBaseColor(graphics::CommandList* command_list)
  {
    base_color_.gpu_address_ = 
      command_list->const_buffer_heap()->Upload(&base_color_.color_.data, sizeof(glm::vec4));
  }
}
