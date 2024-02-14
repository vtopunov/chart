#pragma once

#include <algorithm>

#include <core/span.h>
#include <core/round.h>


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

    template<class R>
    constexpr auto operator += (const vec2<R> &v) noexcept
        -> decltype(_0 += v._0, *this)
    {
        _0 += v._0;
        _1 += v._1;
        return *this;
    }
    
    template<class R>
    constexpr auto operator -= (const vec2<R> &v) noexcept
        -> decltype(_0 -= v._0, *this)
    {
        _0 -= v._0;
        _1 -= v._1;
        return *this;
    }

    template<class R>
    constexpr auto operator *= (const R & v) noexcept
        -> decltype(_0 *= v, *this)
    {
        _0 *= v;
        _1 *= v;
        return *this;
    }

    template<class R>
    constexpr auto operator /= (const R & v) noexcept
        -> decltype(_0 /= v, *this)
    {
        _0 /= v;
        _1 /= v;
        return *this;
    }
};

template<class T>
vec2(T, T) -> vec2<T>;


namespace private_detail_is_base_of_vec2
{
    template<class T, class = void>
    struct is_base_of_vec2_type
    {
        using type = std::false_type;
    };

    template <class T>
    struct is_base_of_vec2_type<T, std::void_t<decl_value_type_t<T>>>
    {
        using type = std::is_base_of<vec2<decl_value_type_t<T>>, T>;
    };
}

template<class T>
using is_base_of_vec2 = typename private_detail_is_base_of_vec2::is_base_of_vec2_type<T>::type;

template<class T>
constexpr auto is_base_of_vec2_v = is_base_of_vec2<T>::value;


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
[[nodiscard]] constexpr auto as_signed(const Vec<T>& v) noexcept -> Vec<decltype(as_signed(as_vec2(v)._0))>
{
    return 
    { 
        as_signed(v._0), 
        as_signed(v._1)
    };
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto as_unsigned(const Vec<T>& v) noexcept -> Vec<decltype(as_unsigned(as_vec2(v)._0))>
{
    return 
    { 
        as_unsigned(v._0), 
        as_unsigned(v._1) 
    };
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

template<class Vec>
[[nodiscard]] constexpr Vec fill_to(const decl_value_type_t<Vec>& value) noexcept
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
[[nodiscard]] constexpr auto inverse(const Vec<T>& v) noexcept -> Vec<decltype(as_vec2(v)._0)>
{
    return
    {
        v._1,
        v._0
    };
}

template<class T>
[[nodiscard]] constexpr auto md_abs(const T& v) noexcept -> decltype(constexpr_abs(v))
{
    return constexpr_abs(v);
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto md_abs(const Vec<T>& v) noexcept -> Vec<decltype(md_abs(as_vec2(v)._0))>
{
    return { md_abs(v._0), md_abs(v._1) };
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_base_of_v<vec2<T>, Vec<T>>, Vec<T>
> md_clamp(const Vec<T>& v, const vec2<T>& v0, const vec2<T>& v1) noexcept
{
    return
    {
        std::clamp(v._0, v0._0, v1._0),
        std::clamp(v._1, v0._1, v1._1),
    };
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto operator - (const Vec<T>& v) noexcept -> Vec<decltype(-(as_vec2(v)._0))>
{
    return
    {
        -v._0,
        -v._1
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator - (const Vec<L>& left, const Vec<R>& right) noexcept -> Vec<decltype(as_vec2(left)._0 - as_vec2(right)._0)>
{
    return
    {
        left._0 - right._0,
        left._1 - right._1
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator + (const Vec<L>& left, const Vec<R>& right) noexcept -> Vec<decltype(as_vec2(left)._0 + as_vec2(right)._0)>
{
    return
    {
        left._0 + right._0,
        left._1 + right._1
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator * (const Vec<L>& left, const R& right) noexcept -> Vec<decltype(as_vec2(left)._0* right)>
{
    return
    {
        left._0 * right,
        left._1 * right
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator * (const L& left, const Vec<R>& right) noexcept -> Vec<decltype(left* as_vec2(right)._0)>
{
    return
    {
        left * right._0,
        left * right._1
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator / (const Vec<L>& left, const R& right) noexcept -> Vec<decltype(as_vec2(left)._0 / right)>
{
    return
    {
        left._0 / right,
        left._1 / right
    };
}

template<class T>
[[nodiscard]] std::enable_if_t<std::is_arithmetic_v<T>, bool> md_isfinite(const T& v) noexcept
{
    return std::isfinite(v);
}

template<class T>
[[nodiscard]] auto md_isfinite(const T& v) noexcept -> decltype(md_isfinite(as_vec2(v)._0))
{
    return md_isfinite(v._0) && md_isfinite(v._1);
}

template<class T>
[[nodiscard]] constexpr auto md_isnormal(const T& v) noexcept -> std::enable_if_t<
    std::negation_v<is_base_of_vec2<T>>,
    decltype(u_isnormal(v))
> 
{
    return u_isnormal(v);
}

template<class T>
[[nodiscard]] constexpr auto md_isnormal(const T& v) noexcept -> decltype(md_isnormal(as_vec2(v)._0))
{
    return md_isnormal(v._0) 
        && md_isnormal(v._1);
}

template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_narrow(const T& v) noexcept
{
    return narrow<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_narrow(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_narrow<value_t>(v._0),
        md_narrow<value_t>(v._1)
    };
}


template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_narrow(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_narrow<value_t>(v0),
        md_narrow<value_t>(v1)
    };
}


template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_numeric_cast(const T& v) noexcept
{
    return numeric_cast<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_numeric_cast(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_numeric_cast<value_t>(v._0),
        md_numeric_cast<value_t>(v._1)
    };
}


template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_numeric_cast(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_numeric_cast<value_t>(v0),
        md_numeric_cast<value_t>(v1)
    };
}


template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_trunc_cast(const T& v) noexcept
{
    return trunc_cast<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_trunc_cast(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_trunc_cast<value_t>(v._0),
        md_trunc_cast<value_t>(v._1)
    };
}

template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_trunc_cast(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_trunc_cast<value_t>(v0),
        md_trunc_cast<value_t>(v1)
    };
}


template<class T, class Near>
[[nodiscard]] constexpr std::enable_if_t<
    std::conjunction_v<std::is_arithmetic<T>, std::is_arithmetic<Near>>,
    Near
> md_round_to_near(T v, Near v_near) noexcept
{
    return round_to_near(v, v_near);
}

template<template<class> class Vec, class T, class Near>
[[nodiscard]] constexpr auto md_round_to_near(const Vec<T>& v, const Vec<Near>& v_near) noexcept -> Vec<decltype(md_round_to_near(as_vec2(v)._0, v_near._0))>
{
    return
    {
        md_round_to_near(v._0, v_near._0),
        md_round_to_near(v._1, v_near._1)
    };
}

template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_arithmetic_v<T>, bool> md_is_safe_narrowing_conversion(const T& v) noexcept
{
    return is_safe_narrowing_conversion<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr auto md_is_safe_narrowing_conversion(const T& v) noexcept -> decltype(is_safe_narrowing_conversion<value_type_t<R>>(as_vec2(v)._0))
{
    using value_t = value_type_t<R>;

    return md_is_safe_narrowing_conversion<value_t>(v._0)
        && md_is_safe_narrowing_conversion<value_t>(v._1);
}


template<class L, class R>
[[nodiscard]] constexpr auto md_min(const L& a, const R& b) noexcept -> decltype(scalar_min(a, b))
{
    return scalar_min(a, b);
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto md_min
(
    const Vec<L>& a,
    const Vec<R>& b
) noexcept -> Vec<decltype(md_min(as_vec2(a)._0, as_vec2(b)._0))>
{
    return
    {
        md_min(a._0, b._0),
        md_min(a._1, b._1)
    };
}

template<class L, class R>
[[nodiscard]] constexpr auto md_max(const L& a, const R& b) noexcept -> decltype(scalar_max(a, b))
{
    return scalar_max(a, b);
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto md_max
(
    const Vec<L>& a,
    const Vec<R>& b
) noexcept -> Vec<decltype(md_max(as_vec2(a)._0, as_vec2(b)._0))>
{
    return
    {
        md_max(a._0, b._0),
        md_max(a._1, b._1)
    };
}