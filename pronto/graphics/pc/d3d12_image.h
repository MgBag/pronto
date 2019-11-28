#pragma once
#include <wtypes.h>
#include <dxgiformat.h>
#include <Windows.h>
#include <wincodec.h>
#include <d3d12.h>
#include <vector>
#include <tiny_gltf.h>

namespace pronto
{
  class Image
  {
  public:
    Image(const tinygltf::Image& image);
    ~Image();

    const unsigned char* data() const;
    unsigned int bit_per_pixel() const;
    unsigned int byte_per_row() const;
    unsigned int image_size() const;
    D3D12_RESOURCE_DESC* resource_desc();
    ID3D12Resource* resource() const;
    void set_resource(ID3D12Resource* resc);
    LPCWSTR& path();
    bool is_valid();
    bool Init(ID3D12Device* device);

  private:
    const tinygltf::Image& image_;
    IWICImagingFactory* wic_factory_;
    LPCWSTR path_;
    const unsigned char* data_;
    int bit_per_pixel_;
    int byte_per_row_;
    int image_size_;
    D3D12_RESOURCE_DESC resource_desc_;
    bool is_valid_;
    ID3D12Resource* resc_;

  };
}
