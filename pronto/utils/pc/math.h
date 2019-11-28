#pragma once

namespace pronto
{
  namespace math
  {
    /***************************************************************************
    * These functions were taken from the MiniEngine.
    * Source code available here:
    * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
    * Retrieved: January 13, 2016
    **************************************************************************/
    template <typename T>
    inline T AlignUpWithMask(T value, size_t mask)
    {
      return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask)
    {
      return (T)((size_t)value & ~mask);
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment)
    {
      return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment)
    {
      return AlignDownWithMask(value, alignment - 1);
    }

    template <typename T>
    inline bool IsAligned(T value, size_t alignment)
    {
      return 0 == ((size_t)value & (alignment - 1));
    }

    template <typename T>
    inline T DivideByMultiple(T value, size_t alignment)
    {
      return (T)((value + alignment - 1) / alignment);
    }
    /***************************************************************************/
  }
}