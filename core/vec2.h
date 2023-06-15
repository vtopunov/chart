#pragma once

#include <core/member_detector.h>
#include <core/size_type.h>
#include <core/span.h>


template<class T>
struct vec2
{
    static constexpr size_t tuple_size{ 2_uz };
    using value_type = T;
    using view_type = span<const T, tuple_size>;

    T _0;
    T _1;

    [[nodiscard]]
    constexpr size_t size() const noexcept
    {
        return tuple_size;
    }

    [[nodiscard]]
    constexpr const T* data() const noexcept
    {
        return std::addressof(_0);
    }

    [[nodiscard]]
    constexpr operator view_type() const noexcept
    {
        return view_type{ data(), tuple_size };
    }

    [[nodiscard]]
    constexpr bool operator == (const vec2&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const vec2&) const noexcept = default;
};

template<class T>
vec2(T, T)->vec2<T>;

template<size_t Index, class T>
[[nodiscard]] constexpr const T& get(const vec2<T>& v) noexcept
{
    if constexpr (0_uz == Index)
    {
        return v._0;
    }
    else
    {
        static_assert(1_uz == Index);
        return v._1;
    }
}

template<size_t Index, class T>
[[nodiscard]] constexpr T& get(vec2<T>& v) noexcept
{
    return as_mutable(get<Index>(std::as_const(v)));
}

template<class T>
[[nodiscard]] constexpr const vec2<T>& as_vec2(const vec2<T>& vec) noexcept
{
    return vec;
}

template<class T>
[[nodiscard]] constexpr vec2<T>& as_vec2(vec2<T>& vec) noexcept
{
    return vec;
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_base_of_v<vec2<T>, Vec<T>>, Vec<remove_unsigned_t<T>>
> as_signed(const Vec<T>& vec) noexcept
{
    return { as_signed(std::move(vec._0)), as_signed(std::move(vec._1)) };
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_base_of_v<vec2<T>, Vec<T>>, Vec<T>> fill_to(const T& value) noexcept
{
    return
    {
        value,
        value
    };
}

template<class T>
[[nodiscard]] constexpr vec2<T> fill_vec2(const T& value) noexcept
{
    return fill_to<vec2>(value);
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr std::enable_if_t <
    std::is_base_of_v<vec2<T>, Vec<T>>, Vec<T>
> inverse(const Vec<T>& v) noexcept
{
    return
    {
        v._1,
        v._0
    };
}


template<class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::negation_v<std::is_unsigned<T>>, vec2<std::remove_cvref_t<decltype(-std::declval<std::add_const_t<T>>())>>
> operator - (const vec2<T>& right) noexcept
{
    return
    {
        -right._0,
        -right._1
    };
}

template<class L, class R>
[[nodiscard]] constexpr decltype(auto) operator - (const vec2<L>& left, const vec2<R>& right) noexcept
{
    using common_t = std::remove_cvref_t<decltype(left._0 - right._0)>;
    using common_vec2_t = vec2<common_t>;

    return common_vec2_t
    {
        left._0 - right._0,
        left._1 - right._1
    };
}

template<class L, class R>
[[nodiscard]] constexpr decltype(auto) operator + (const vec2<L>& left, const vec2<R>& right) noexcept
{
    using common_t = std::remove_cvref_t<decltype(left._0 + right._0)>;
    using common_vec2_t = vec2<common_t>;

    return common_vec2_t
    {
        left._0 + right._0,
        left._1 + right._1
    };
}

template<class L, class R>
using decl_mul_t = std::remove_cvref_t<decltype(std::declval<std::add_const_t<L>>()* std::declval<std::add_const_t<R>>())>;

template<class L, class R>
using decl_div_t = std::remove_cvref_t<decltype(std::declval<std::add_const_t<L>>() / std::declval<std::add_const_t<R>>())>;

template<class T>
using is_salar_for_vec = std::negation<is_data_pointer<T>>;

template<class T>
constexpr bool is_salar_for_vec_v = is_salar_for_vec<T>::value;

template<class T>
[[nodiscard]] constexpr vec2<decl_mul_t<T, T>> operator * (const vec2<T>& left, const T& right) noexcept
{
    return
    {
        left._0 * right,
        left._1 * right
    };
}

template<class T>
[[nodiscard]] constexpr decltype(auto) operator * (const T& left, const vec2<T>& right) noexcept
{
    return right * left;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<
    is_salar_for_vec_v<T>,
    vec2<decl_div_t<T, T>>
> operator / (const vec2<T>& left, const T& right) noexcept
{
    return
    {
        left._0 / right,
        left._1 / right
    };
}

namespace private_detail_vec2
{
    template<class VecScalar, class Scalar>
    constexpr bool is_compatible_scalar_for_vec0_v = std::conjunction_v<
        is_salar_for_vec<Scalar>,
        std::negation<std::is_base_of<VecScalar, Scalar>>
    >;
}

template<class VecScalar, class Scalar>
constexpr bool is_compatible_scalar_for_vec_v = private_detail_vec2::is_compatible_scalar_for_vec0_v
<
    std::remove_cvref_t<VecScalar>, std::remove_cvref_t<Scalar>
>;

template<class T, class U>
[[nodiscard]] constexpr std::enable_if_t<
    is_compatible_scalar_for_vec_v<T, U>,
    vec2<decl_mul_t<T, U>>
> operator * (const vec2<T>& left, const U& right) noexcept
{
    return
    {
        left._0 * right,
        left._1 * right
    };
}

template<class U, class T>
[[nodiscard]] constexpr auto operator * (const U& left, const vec2<T>& right) noexcept -> decltype(right* left)
{
    return right * left;
}

template<class T, class U>
[[nodiscard]] constexpr std::enable_if_t<
    is_compatible_scalar_for_vec_v<T, U>,
    vec2<decl_div_t<T, U>>
>  operator / (const vec2<T>& left, const U& right) noexcept
{
    return
    {
        left._0 / right,
        left._1 / right
    };
}

template<class OutT, class InT>
[[nodiscard]] constexpr OutT narrow2d_cast(InT x, InT y) noexcept
{
    using value_t = value_type_t<OutT>;

    return
    {
        narrow_cast<value_t>(std::move(x)),
        narrow_cast<value_t>(std::move(y))
    };
}

template<class OutT, class InT>
[[nodiscard]] constexpr OutT narrow2d_cast(vec2<InT> in) noexcept
{
    return narrow2d_cast<OutT>(std::move(in._0), std::move(in._1));
}
