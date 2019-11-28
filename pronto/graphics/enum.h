#pragma once

namespace pronto
{
  namespace graphics
  {
    // TODO: Perhaps note that there is a differnce between a texture with a color and vertex colors
    enum struct PrimitiveType : int
    {
      kColor = 0,
      kTexture,
      kTransparantTexture,
      kNormalMap,
      kMultipleUV,
      kShadow,
      kCount
    };
  }
}