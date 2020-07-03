#pragma once

#include <utility>

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

template <class T>
constexpr T& as_mutable(const T& value) noexcept
{   
    return const_cast<T&>(value);
}

template <class T>
constexpr T* as_mutable_pointer(const T* value) noexcept
{
    return const_cast<T*>(value);
}