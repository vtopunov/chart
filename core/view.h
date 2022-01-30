#pragma once

#include <core/member_detector.h>

template<class T>
using decl_view_t = typename T::view_type;

template<class T>
using default_view_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

template <class T>
using view_t = detected_or_t<default_view_t<T>, decl_view_t, T>;

template <class T>
inline constexpr bool is_view_v = is_detected_v<decl_view_t, T>;

template<class T>
constexpr view_t<T> view(const T& r) noexcept
{
    D_WARNING_PUSH
        D_WARNING_DISABLE_MSVC(W_do_not_slice)
        return static_cast<view_t<T>>(r);
    D_WARNING_POP
}

