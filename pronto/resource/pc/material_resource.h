#pragma once
#include <string>
#include <dxgiformat.h>
#include <glm/vec4.hpp>
#include <d3d12.h>
#include "graphics/enum.h"

namespace pronto
{
  namespace graphics
  {
    class CommandList;
  }

  struct ImageBuffer
  {
    ImageBuffer() :
      buffer_(nullptr), is_transparant_(false) {}
    const unsigned char* buffer_;
    size_t stride_;
    size_t count_;
    size_t channel_depth_;
    size_t component_;
    DXGI_FORMAT format_;
    size_t height_;
    size_t width_;
    bool is_transparant_;
    ID3D12Resource* resource_;
  };

  // Maybe this needs a is transparant thingy 
  struct BaseColor
  {
    BaseColor() : is_valid_(false) {}
    bool is_valid_;
    glm::vec4 color_;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_address_;
  };

  class MaterialResource
  {
  public:
    MaterialResource();

    MaterialResource(
      const std::string& path,
      unsigned char* texture,
      size_t texture_stride,
      size_t texture_count,
      DXGI_FORMAT texture_format,
      size_t height_,
      size_t width_);

    ~MaterialResource();

    const std::string& path() const;
    void path(const std::string& path);

    ImageBuffer& texture();
    ImageBuffer& normal_map();
    BaseColor& base_color();
    graphics::PrimitiveType type();

    void texture(const ImageBuffer& texture);
    void normal_map(const ImageBuffer& normal_map);
    void base_color(const BaseColor& base_color);
    void base_color(const glm::vec4& color);
    void type(graphics::PrimitiveType type);

    void Upload(graphics::CommandList* command_list);

  private:
    std::string path_;
    graphics::PrimitiveType type_;

    ImageBuffer texture_;
    ImageBuffer normal_map_;
    BaseColor base_color_;

    bool is_transparant_;

    void UploadBuffer(graphics::CommandList* command_list, ImageBuffer& buffer);
    void UploadBaseColor(graphics::CommandList* command_list);
  };
}
