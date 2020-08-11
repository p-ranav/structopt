#pragma once
#include <array>
using std::array;

namespace argo {

template<typename>
struct array_size;
template<typename T, size_t N>
struct array_size<array<T,N> > {
  static size_t const size = N;
};

}