#include "d3d12_image.h"

#pragma warning(push, 3) 
#include "graphics/pc/d3dx12.h"
#pragma warning(pop) 

#include <assert.h>
#include <iostream>

#include "platform/conversions.h"

namespace pronto
{
  Image::Image(const tinygltf::Image& image) :
    wic_factory_(nullptr),
    path_(L""),
    data_(nullptr),
    bit_per_pixel_(0),
    byte_per_row_(0),
    image_size_(0),
    is_valid_(false),
    resource_desc_(),
    image_(image)
  {
  }

  Image::~Image()
  {
  }

  bool Image::Init(ID3D12Device* device)
  {
    IWICBitmapFrameDecode* wic_frame = NULL;

    bool image_converted = false;
    IWICFormatConverter* wic_converter = NULL;

    // convert wic pixel format to dxgi pixel format
    DXGI_FORMAT dxgi_format = ToDXGIFormat(image_.component, image_.pixel_type, true /*normalized*/);

    assert(dxgi_format != DXGI_FORMAT_UNKNOWN);

    byte_per_row_ = image_.width * image_.bits * image_.component / 8; // number of bytes in each row of the image data
    image_size_ = byte_per_row_ * image_.height; // total image size in bytes

    // allocate enough memory for the raw image data, and set imageData to point to that memory
    data_ = image_.image.data();

    // now describe the texture with the information we have obtained from the image
    resource_desc_ = {};
    resource_desc_.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc_.Alignment = 0;
    // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB 
    //(4MB for multi-sampled textures)
    resource_desc_.Width = image_.width;
    // width of the texture
    resource_desc_.Height = image_.height;
    // height of the texture
    resource_desc_.DepthOrArraySize = 1;
    // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures
    //(we only have one image, so we set 1)
    resource_desc_.MipLevels = 1;
    // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
    resource_desc_.Format = dxgi_format;
    // This is the dxgi format of the image (format of the pixels)
    resource_desc_.SampleDesc.Count = 1;
    // This is the number of samples per pixel, we just want 1 sample
    resource_desc_.SampleDesc.Quality = 0;
    // The quality level of the samples. Higher is better quality, but worse performance
    resource_desc_.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
    resource_desc_.Flags = D3D12_RESOURCE_FLAG_NONE;
    // no flags

    device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
      D3D12_HEAP_FLAG_NONE,
      &resource_desc_,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(&resc_));

    resc_->SetName(L"Texture resource");

    is_valid_ = true;

    return true;
  }

  const unsigned char* Image::data() const
  {
    return data_;
  }

  unsigned int Image::bit_per_pixel() const
  {
    return bit_per_pixel_;
  }

  unsigned int Image::byte_per_row() const
  {
    return byte_per_row_;
  }

  unsigned int Image::image_size() const
  {
    return image_size_;
  }

  D3D12_RESOURCE_DESC* Image::resource_desc()
  {
    return &resource_desc_;
  }

  ID3D12Resource* Image::resource() const
  {
    return resc_;
  }

  void Image::set_resource(ID3D12Resource* resc)
  {
    resc_ = resc;
  }

  LPCWSTR& Image::path()
  {
    return path_;
  }

  bool Image::is_valid()
  {
    return is_valid_;
  }
}
