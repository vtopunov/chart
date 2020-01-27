#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <limits>
#include <numeric>

template<class T>
constexpr T max_v = std::numeric_limits<T>::max();

template<class T>
constexpr T lowest_v = std::numeric_limits<T>::lowest();

struct empty {};