#pragma once
#include "allocator.h"
#include <vector>
#include <list>
#include <string>
#include <map>

namespace pronto
{
#ifdef PWIN
  template <typename T>
  using pvector = std::vector<T>;

  typedef std::string pstring;

  template <typename T, typename Y>
  using pmap = std::map<T, Y>;

  template <typename T>
  using plist = std::list<T>;

#else
  template <typename T>
  using RVector = std::vector<T, CAllocator<T>>;

  template <typename T>
  using RList = std::list<T, CAllocator<T>>;

  typedef std::basic_string<char, std::char_traits<char>, CAllocator<char>> RString;

  template <typename T, typename Y>
  using RMap = std::map<T, Y, std::less<T>, CAllocator<std::pair<T, Y>>>;
#endif
}
