#pragma once
#include <wtypes.h>
#include <dxgiformat.h>
#include <Windows.h>
#include <wincodec.h>
#include <d3d12.h>

namespace pronto
{
  class Texture
  {
  public:
    Texture(LPCWSTR path);
    ~Texture();

    bool Init();

    unsigned char* data();
    unsigned int bit_per_pixel();
    unsigned int byte_per_row();
    unsigned int image_size();
    D3D12_RESOURCE_DESC* resource_desc();
    LPCWSTR& path();
    bool is_valid();

  private:
    IWICImagingFactory* wic_factory_;
    LPCWSTR path_;
    unsigned char* data_;
    int bit_per_pixel_;
    int byte_per_row_;
    int image_size_;
    D3D12_RESOURCE_DESC resource_desc_;
    bool is_valid_;

    DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& format);
    WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& format);
    int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& format);
  };
}
