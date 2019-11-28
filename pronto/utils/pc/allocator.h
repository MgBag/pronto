#pragma once
#include <memory>
#include <inttypes.h>
#include <assert.h>

namespace pronto
{
  template<typename T>
  class CAllocator
  {
  public:
    typedef T value_type;

    CAllocator() noexcept {} //default ctor not required by STL  

    T* allocate(const size_t n) const;
    void deallocate(T* const p, size_t) const noexcept;

    // A converting copy constructor:  
    template<class U> CAllocator(const CAllocator<U>&) noexcept {}

    template<class U> bool operator==(const CAllocator<U>&) const noexcept
    {
      return true;
    }
    template<class U> bool operator!=(const CAllocator<U>&) const noexcept
    {
      return false;
    }
  };
   
  template <class T>
  T* CAllocator<T>::allocate(const size_t n) const
  {
    return static_cast<T*>(malloc(n));
  }

  template<class T>
  void CAllocator<T>::deallocate(T * const p, size_t s) const noexcept
  {
    free(p);
  }

  template <class T, class ... Args>
  inline T* pnew(Args... args)
  {
    return new T(args...);
  }

  template<class T>
  void pdelete(T* p)
  {
    delete p;
  }

  inline void* pmalloc(size_t size)
  {
    return malloc(size);
  }

  inline void pfree(void* ptr)
  {
    ::free(ptr);
  }

  template<class T>
  T* pnewa(size_t length)
  {
    return new T[length];
  }

  template<class T>
  void pdeletea(T* arr)
  {
    delete[] arr;
  }
}