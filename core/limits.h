#pragma once

#include <limits>

#undef min
#undef max


template<class T>
constexpr auto numeric_max_v = std::numeric_limits<T>::max();

template<class T>
constexpr auto numeric_min_v = std::numeric_limits<T>::min();

template<class T>
constexpr auto numeric_eps_v = std::numeric_limits<T>::epsilon();

template<class T>
constexpr auto numeric_lowest_v = std::numeric_limits<T>::lowest();

template<class T>
constexpr auto numeric_nan_v = std::numeric_limits<T>::quiet_NaN();

template<class T>
constexpr auto numeric_inf_v = std::numeric_limits<T>::infinity();

template<class T>
constexpr auto numeric_digits_v = std::numeric_limits<T>::digits;