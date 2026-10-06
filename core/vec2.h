#pragma once

#include <algorithm>

#include <core/tuple.h>
#include <core/round.h>


template<class T>
struct vec2 : tuple<T, T>
{
    static constexpr size_t extent{ has_no_unique_address_v<T> ? 1u : 2u };

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using tuple_type = tuple<T, T>;
    using tuple_type::_0;
    using tuple_type::_1;

    [[nodiscard]]
    constexpr size_t size() const noexcept
    {
        return extent;
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return std::addressof(_0);
    }

    D_DEFAULT_EQ_OP(vec2);

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

template<class T>
using decl_as_vec2_t = decltype(as_vec2(std::declval<T&>()));

template<class T>
using is_detected_as_vec2 = is_detected<decl_as_vec2_t, const T>;

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
[[nodiscard]] constexpr auto md_as_signed(const T& v) noexcept -> decltype(as_signed(v))
{
    return as_signed(v);
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto md_as_signed(const Vec<T>& v) noexcept -> Vec<decltype(md_as_signed(as_vec2(v)._0))>
{
    return
    {
        md_as_signed(v._0),
        md_as_signed(v._1)
    };
}

template<class T>
[[nodiscard]] constexpr auto md_as_unsigned(const T& v) noexcept -> decltype(as_unsigned(v))
{
    return as_unsigned(v);
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto md_as_unsigned(const Vec<T>& v) noexcept -> Vec<decltype(md_as_unsigned(as_vec2(v)._0))>
{
    return
    {
        md_as_unsigned(v._0),
        md_as_unsigned(v._1)
    };
}

template<class T>
[[nodiscard]] constexpr auto md_abs(const T& v) noexcept -> decltype(u_abs(v))
{
    return u_abs(v);
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto md_abs(const Vec<T>& v) noexcept -> Vec<decltype(md_abs(as_vec2(v)._0))>
{
    return 
    { 
        md_abs(v._0), 
        md_abs(v._1)
    };
}

template<class T>
[[nodiscard]] constexpr auto md_round(const T& v) noexcept -> std::enable_if_t<
    std::is_arithmetic_v<T>, 
    decltype(std::round(v))
>
{
    return std::round(v);
}

template<template<class> class Vec, class T>
[[nodiscard]] constexpr auto md_round(const Vec<T>& v) noexcept -> Vec<decltype(md_round(as_vec2(v)._0))>
{
    return 
    { 
        md_round(v._0), 
        md_round(v._1)
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
    return md_isfinite(v._0) 
        && md_isfinite(v._1);
}

template<class T>
[[nodiscard]] constexpr auto md_isnormal(const T& v) noexcept -> std::enable_if_t<
    std::negation_v<is_detected_as_vec2<T>>,
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

template<class T>
[[nodiscard]] constexpr auto md_is_neqnz(const T& v) noexcept -> std::enable_if_t<
    std::negation_v<is_detected_as_vec2<T>>,
    decltype(is_neqnz(v))
>
{
    return is_neqnz(v);
}

template<class T>
[[nodiscard]] constexpr auto md_is_neqnz(const T& v) noexcept -> decltype(md_is_neqnz(as_vec2(v)._0))
{
    return md_is_neqnz(v._0)
        && md_is_neqnz(v._1);
}

template<class T>
[[nodiscard]] constexpr auto md_is_eqnz(const T& v) noexcept -> std::enable_if_t<
    std::negation_v<is_detected_as_vec2<T>>,
    decltype(is_eqnz(v))
>
{
    return is_eqnz(v);
}

template<class T>
[[nodiscard]] constexpr auto md_is_eqnz(const T& v) noexcept -> decltype(md_is_eqnz(as_vec2(v)._0))
{
    return md_is_eqnz(v._0)
        && md_is_eqnz(v._1);
}

template<class T>
[[nodiscard]] constexpr auto md_is_positiven(const T& v) noexcept -> decltype(is_positiven(v))
{
    return is_positiven(v);
}

template<class T>
[[nodiscard]] constexpr auto md_is_positiven(const T& v) noexcept -> decltype(md_is_positiven(as_vec2(v)._0))
{
    return md_is_positiven(v._0)
        && md_is_positiven(v._1);
}

template<class T>
[[nodiscard]] constexpr auto md_is_negativen(const T& v) noexcept -> decltype(is_negativen(v))
{
    return is_negativen(v);
}

template<class T>
[[nodiscard]] constexpr auto md_is_negativen(const T& v) noexcept -> decltype(md_is_negativen(as_vec2(v)._0))
{
    return md_is_negativen(v._0)
        && md_is_negativen(v._1);
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


template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_round_cast(const T& v) noexcept
{
    return round_cast<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_round_cast(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_round_cast<value_t>(v._0),
        md_round_cast<value_t>(v._1)
    };
}

template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_round_cast(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_round_cast<value_t>(v0),
        md_round_cast<value_t>(v1)
    };
}


template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_floor_cast(const T& v) noexcept
{
    return floor_cast<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_floor_cast(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_floor_cast<value_t>(v._0),
        md_floor_cast<value_t>(v._1)
    };
}

template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_floor_cast(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_floor_cast<value_t>(v0),
        md_floor_cast<value_t>(v1)
    };
}


template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_ceil_cast(const T& v) noexcept
{
    return ceil_cast<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_ceil_cast(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_ceil_cast<value_t>(v._0),
        md_ceil_cast<value_t>(v._1)
    };
}

template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_ceil_cast(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_ceil_cast<value_t>(v0),
        md_ceil_cast<value_t>(v1)
    };
}


template<class R, class T>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_base_of_vec2<T>>, R> md_clamp_cast(const T& v) noexcept
{
    return clamp_cast<R>(v);
}

template<class R, class T>
[[nodiscard]] constexpr R md_clamp_cast(const vec2<T>& v) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_clamp_cast<value_t>(v._0),
        md_clamp_cast<value_t>(v._1)
    };
}

template<class R, class T0, class T1>
[[nodiscard]] constexpr R md_clamp_cast(const T0& v0, const T1& v1) noexcept
{
    using value_t = value_type_t<R>;

    return
    {
        md_clamp_cast<value_t>(v0),
        md_clamp_cast<value_t>(v1)
    };
}


template<class T, class Near>
[[nodiscard]] constexpr std::enable_if_t<
    std::conjunction_v<std::is_arithmetic<T>, std::is_arithmetic<Near>>,
    Near
> md_round_to_near(const T& v, const Near& v_near) noexcept
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
[[nodiscard]] constexpr auto md_min(const L& a, const R& b) noexcept -> decltype(u_min(a, b))
{
    return u_min(a, b);
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
[[nodiscard]] constexpr auto md_max(const L& a, const R& b) noexcept -> decltype(u_max(a, b))
{
    return u_max(a, b);
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