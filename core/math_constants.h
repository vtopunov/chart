#pragma once

#include <type_traits>

template<typename T>
struct math_constants
{
    static_assert(std::is_floating_point<T>::value,
        "template argument not a floating point type");

    static constexpr T pi = 3.1415926535897932384626433832795029L;
    static constexpr T e = 2.7182818284590452353602874713526625L;
};


template<class T> 
constexpr T e_v = math_constants<T>::e;

template<class T>
constexpr T pi_v = math_constants<T>::pi;