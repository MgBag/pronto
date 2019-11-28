#pragma once

#ifdef PWIN
#include "core/input/pc_input.h"
namespace pronto
{
  using Input = InputWin32;
}
#elif PPS4
#include "utils/pc/ps4_allocator.h"
#endif // PWIN
#pragma once
