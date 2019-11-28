#pragma once

#ifdef PWIN
#include "graphics/pc/d3d12_renderer.h"
#elif PPS4
#endif

namespace pronto
{
  namespace graphics
  {
#ifdef PWIN
    using PlatformRenderer = D3D12Renderer;
#elif PPS4
#endif
  }
}