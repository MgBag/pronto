#include "texture.h"

#pragma warning(push, 3) 
#include "graphics/pc/d3dx12.h"
#pragma warning(pop) 

#include <assert.h>
#include <iostream>

namespace pronto
{
  Texture::Texture(LPCWSTR path) :
    wic_factory_(nullptr),
    path_(path),
    data_(nullptr),
    bit_per_pixel_(0),
    byte_per_row_(0),
    image_size_(0),
    is_valid_(false)
  {
  }

  Texture::~Texture()
  {
  }

  bool Texture::Init()
  {
    HRESULT hr;

    hr = ::CoInitializeEx(NULL, 0);

    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false && "We failed m8");
      return false;
    }

    hr = ::CoCreateInstance(
      CLSID_WICImagingFactory,
      NULL,
      CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(&wic_factory_));

    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false && "We failed m8");
      return false;
    }

    // (BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow)

    IWICBitmapDecoder* wic_decoder = NULL;

    // load a decoder for the image
    hr = wic_factory_->CreateDecoderFromFilename(
      path_,                             // Image we want to load in
      NULL,                             // This is a vendor ID, we do not prefer a specific one so set to null
      GENERIC_READ,                     // We want to read from this file
      WICDecodeMetadataCacheOnLoad,     // We will cache the metadata right away, rather than when needed, which might be unknown
      &wic_decoder                      // the wic decoder to be created
    );
    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false && "We failed m8");
      return false;
    }

    IWICBitmapFrameDecode* wic_frame = NULL;
    hr = wic_decoder->GetFrame(0, &wic_frame);
    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false && "We failed m8");
      return false;
    }

    // get wic pixel format of image
    WICPixelFormatGUID pixel_format;
    hr = wic_frame->GetPixelFormat(&pixel_format);
    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false && "We failed m8");
      return false;
    }

    // get size of image
    UINT tex_width, tex_height;
    hr = wic_frame->GetSize(&tex_width, &tex_height);
    if (FAILED(hr))
    {
      DWORD error = ::GetLastError();
      assert(false && "We failed m8");
      return false;
    }

    bool image_converted = false;
    IWICFormatConverter* wic_converter = NULL;

    // convert wic pixel format to dxgi pixel format
    DXGI_FORMAT dxgi_format = GetDXGIFormatFromWICFormat(pixel_format);

    // if the format of the image is not a supported dxgi format, try to convert it
    if (dxgi_format == DXGI_FORMAT_UNKNOWN)
    {
      // get a dxgi compatible wic format from the current image format
      WICPixelFormatGUID convert_to_pixel_format = GetConvertToWICFormat(pixel_format);

      // return if no dxgi compatible format was found
      if (convert_to_pixel_format == GUID_WICPixelFormatDontCare) return 0;

      // set the dxgi format
      dxgi_format = GetDXGIFormatFromWICFormat(convert_to_pixel_format);

      // create the format converter
      hr = wic_factory_->CreateFormatConverter(&wic_converter);
      if (FAILED(hr))
      {
        DWORD error = ::GetLastError();
        assert(false && "We failed m8");
        return false;
      }

      // make sure we can convert to the dxgi compatible format
      BOOL can_convert = FALSE;
      hr = wic_converter->CanConvert(pixel_format, convert_to_pixel_format, &can_convert);
      if (FAILED(hr))
      {
        DWORD error = ::GetLastError();
        assert(false && "We failed m8");
        return false;
      }

      // do the conversion (wicConverter will contain the converted image)
      hr = wic_converter->Initialize(
        wic_frame,
        convert_to_pixel_format,
        WICBitmapDitherTypeErrorDiffusion,
        0,
        0,
        WICBitmapPaletteTypeCustom);
      if (FAILED(hr))
      {
        DWORD error = ::GetLastError();
        assert(false && "We failed m8");
        return false;
      }

      image_converted = true;
    }

    bit_per_pixel_ = GetDXGIFormatBitsPerPixel(dxgi_format); // number of bits per pixel
    byte_per_row_ = (tex_width * bit_per_pixel_) / 8; // number of bytes in each row of the image data
    image_size_ = byte_per_row_ * tex_height; // total image size in bytes

    // allocate enough memory for the raw image data, and set imageData to point to that memory
    data_ = static_cast<unsigned char*>(malloc(image_size_));

    // copy (decoded) raw image data into the newly allocated memory (imageData)
    if (image_converted)
    {
      // if image format needed to be converted, the wic converter will contain the converted image
      hr = wic_converter->CopyPixels(0, byte_per_row_, image_size_, data_);
      if (FAILED(hr)) return 0;
    }
    else
    {
      // no need to convert, just copy data from the wic frame
      hr = wic_frame->CopyPixels(0, byte_per_row_, image_size_, data_);
      if (FAILED(hr)) return 0;
    }

    // now describe the texture with the information we have obtained from the image
    resource_desc_ = {};
    resource_desc_.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc_.Alignment = 0;
    // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB 
    //(4MB for multi-sampled textures)
    resource_desc_.Width = tex_width;
    // width of the texture
    resource_desc_.Height = tex_height;
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

    is_valid_ = true;

    return true;
  }

  unsigned char* Texture::data()
  {
    return data_;
  }

  unsigned int Texture::bit_per_pixel()
  {
    return bit_per_pixel_;
  }

  unsigned int Texture::byte_per_row()
  {
    return byte_per_row_;
  }

  unsigned int Texture::image_size()
  {
    return image_size_;
  }

  D3D12_RESOURCE_DESC* Texture::resource_desc()
  {
    return &resource_desc_;
  }

  LPCWSTR& Texture::path()
  {
    return path_;
  }

  bool Texture::is_valid()
  {
    return is_valid_;
  }

  // get the dxgi format equivilent of a wic format
  //-----------------------------------------------------------------------------------------------
  DXGI_FORMAT Texture::GetDXGIFormatFromWICFormat(WICPixelFormatGUID& format)
  {
    if (format == GUID_WICPixelFormat128bppRGBAFloat)
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
    else if (format == GUID_WICPixelFormat64bppRGBAHalf)
      return DXGI_FORMAT_R16G16B16A16_FLOAT;
    else if (format == GUID_WICPixelFormat64bppRGBA)
      return DXGI_FORMAT_R16G16B16A16_UNORM;
    else if (format == GUID_WICPixelFormat32bppRGBA)
      return DXGI_FORMAT_R8G8B8A8_UNORM;
    else if (format == GUID_WICPixelFormat32bppBGRA)
      return DXGI_FORMAT_B8G8R8A8_UNORM;
    else if (format == GUID_WICPixelFormat32bppBGR)
      return DXGI_FORMAT_B8G8R8X8_UNORM;
    else if (format == GUID_WICPixelFormat32bppRGBA1010102XR)
      return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
    else if (format == GUID_WICPixelFormat32bppRGBA1010102)
      return DXGI_FORMAT_R10G10B10A2_UNORM;
    else if (format == GUID_WICPixelFormat16bppBGRA5551)
      return DXGI_FORMAT_B5G5R5A1_UNORM;
    else if (format == GUID_WICPixelFormat16bppBGR565)
      return DXGI_FORMAT_B5G6R5_UNORM;
    else if (format == GUID_WICPixelFormat32bppGrayFloat)
      return DXGI_FORMAT_R32_FLOAT;
    else if (format == GUID_WICPixelFormat16bppGrayHalf)
      return DXGI_FORMAT_R16_FLOAT;
    else if (format == GUID_WICPixelFormat16bppGray)
      return DXGI_FORMAT_R16_UNORM;
    else if (format == GUID_WICPixelFormat8bppGray)
      return DXGI_FORMAT_R8_UNORM;
    else if (format == GUID_WICPixelFormat8bppAlpha)
      return DXGI_FORMAT_A8_UNORM;
    return DXGI_FORMAT_UNKNOWN;
  }

  // get a dxgi compatible wic format from another wic format
  WICPixelFormatGUID Texture::GetConvertToWICFormat(WICPixelFormatGUID& format)
  {
    if (format == GUID_WICPixelFormatBlackWhite)
      return GUID_WICPixelFormat8bppGray;
    else if (format == GUID_WICPixelFormat1bppIndexed)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat2bppIndexed)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat4bppIndexed)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat8bppIndexed)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat2bppGray)
      return GUID_WICPixelFormat8bppGray;
    else if (format == GUID_WICPixelFormat4bppGray)
      return GUID_WICPixelFormat8bppGray;
    else if (format == GUID_WICPixelFormat16bppGrayFixedPoint)
      return GUID_WICPixelFormat16bppGrayHalf;
    else if (format == GUID_WICPixelFormat32bppGrayFixedPoint)
      return GUID_WICPixelFormat32bppGrayFloat;
    else if (format == GUID_WICPixelFormat16bppBGR555)
      return GUID_WICPixelFormat16bppBGRA5551;
    else if (format == GUID_WICPixelFormat32bppBGR101010)
      return GUID_WICPixelFormat32bppRGBA1010102;
    else if (format == GUID_WICPixelFormat24bppBGR)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat24bppRGB)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat32bppPBGRA)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat32bppPRGBA)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat48bppRGB)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat48bppBGR)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat64bppBGRA)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat64bppPRGBA)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat64bppPBGRA)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat48bppRGBFixedPoint)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat48bppBGRFixedPoint)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat64bppRGBAFixedPoint)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat64bppBGRAFixedPoint)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat64bppRGBFixedPoint)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat64bppRGBHalf)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat48bppRGBHalf)
      return GUID_WICPixelFormat64bppRGBAHalf;
    else if (format == GUID_WICPixelFormat128bppPRGBAFloat)
      return GUID_WICPixelFormat128bppRGBAFloat;
    else if (format == GUID_WICPixelFormat128bppRGBFloat)
      return GUID_WICPixelFormat128bppRGBAFloat;
    else if (format == GUID_WICPixelFormat128bppRGBAFixedPoint)
      return GUID_WICPixelFormat128bppRGBAFloat;
    else if (format == GUID_WICPixelFormat128bppRGBFixedPoint)
      return GUID_WICPixelFormat128bppRGBAFloat;
    else if (format == GUID_WICPixelFormat32bppRGBE)
      return GUID_WICPixelFormat128bppRGBAFloat;
    else if (format == GUID_WICPixelFormat32bppCMYK)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat64bppCMYK)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat40bppCMYKAlpha)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat80bppCMYKAlpha)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat32bppRGB)
      return GUID_WICPixelFormat32bppRGBA;
    else if (format == GUID_WICPixelFormat64bppRGB)
      return GUID_WICPixelFormat64bppRGBA;
    else if (format == GUID_WICPixelFormat64bppPRGBAHalf)
      return GUID_WICPixelFormat64bppRGBAHalf;
    return GUID_WICPixelFormatDontCare;
  }

  // get the number of bits per pixel for a dxgi format
  int Texture::GetDXGIFormatBitsPerPixel(DXGI_FORMAT& format)
  {
    if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
      return 128;
    else if (format == DXGI_FORMAT_R16G16B16A16_FLOAT)
      return 64;
    else if (format == DXGI_FORMAT_R16G16B16A16_UNORM)
      return 64;
    else if (format == DXGI_FORMAT_R8G8B8A8_UNORM)
      return 32;
    else if (format == DXGI_FORMAT_B8G8R8A8_UNORM)
      return 32;
    else if (format == DXGI_FORMAT_B8G8R8X8_UNORM)
      return 32;
    else if (format == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM)
      return 32;
    else if (format == DXGI_FORMAT_R10G10B10A2_UNORM)
      return 32;
    else if (format == DXGI_FORMAT_B5G5R5A1_UNORM)
      return 16;
    else if (format == DXGI_FORMAT_B5G6R5_UNORM)
      return 16;
    else if (format == DXGI_FORMAT_R32_FLOAT)
      return 32;
    else if (format == DXGI_FORMAT_R16_FLOAT)
      return 16;
    else if (format == DXGI_FORMAT_R16_UNORM)
      return 16;
    else if (format == DXGI_FORMAT_R8_UNORM)
      return 8;
    else if (format == DXGI_FORMAT_A8_UNORM)
      return 8;
    return 0;
  }
}
