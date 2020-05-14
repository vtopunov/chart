#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <limits>
#include <numeric>
#include <compare>

#undef min
#undef max

template<class T>
inline constexpr T max_v = std::numeric_limits<T>::max();

template<class T>
inline constexpr T lowest_v = std::numeric_limits<T>::lowest();

struct empty {};

template<class T>
struct add_const_to_pointer
{
    using type = T;
};

template<class T>
struct add_const_to_pointer<T*>
{
    using type = const T*;
};

template<class T>
using add_const_to_pointer_t = typename add_const_to_pointer<T>::type;

template<class T>
constexpr bool is_null_or_empty(const T* string) noexcept
{
    return !string || !*string;
}

template <class T>
constexpr T& as_mutable(const T& value) noexcept
{   
    return const_cast<T&>(value);
}