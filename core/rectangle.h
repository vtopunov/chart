#pragma once

#include <core/point2d.h>
#include <core/size2d.h>


template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator - (const LVec<L>& left, const RVec<R>& right) noexcept -> LVec<decltype(as_point2d(left)._0 - as_size2d(right)._0)>
{
    return { as_vec2(left) - as_vec2(right) };
}

template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator - (const LVec<L>& left, const RVec<R>& right) noexcept -> RVec<decltype(as_size2d(left)._0 - as_point2d(right)._0)>
{
    return { as_vec2(left) - as_vec2(right) };
}


template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator + (const LVec<L>& left, const RVec<R>& right) noexcept -> LVec<decltype(as_point2d(left)._0 + as_size2d(right)._0)>
{
    return { as_vec2(left) + as_vec2(right) };
}

template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator + (const LVec<L>& left, const RVec<R>& right) noexcept -> RVec<decltype(as_size2d(left)._0 + as_point2d(right)._0)>
{
    return { as_vec2(left) + as_vec2(right) };
}

template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator * (const LVec<L>& left, const RVec<R>& right) noexcept -> LVec<decltype(as_point2d(left)._0* as_size2d(right)._0)>
{
    return
    {
        left._0 * right._0,
        left._1 * right._1
    };
}

template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator * (const LVec<L>& left, const RVec<R>& right) noexcept -> RVec<decltype(as_size2d(left)._0* as_point2d(right)._0)>
{
    return
    {
        left._0 * right._0,
        left._1 * right._1
    };
}

template<template<class> class LVec, class L, template<class> class RVec, class R>
[[nodiscard]] constexpr auto operator / (const LVec<L>& left, const RVec<R>& right) noexcept -> LVec<decltype(as_point2d(left)._0 / as_size2d(right)._0)>
{
    return
    {
        left._0 / right._0,
        left._1 / right._1
    };
}


template<class Value, class Size = unsigned_or_t<Value>>
struct rectangle
{
    using value_type = Value;
    using size_type = Size;
    using point2d_type = point2d<value_type>;
    using size2d_type = size2d<size_type>;

    point2d_type position;
    size2d_type sizes;

    [[nodiscard]]
    constexpr point2d_type p00() const noexcept
    {
        return position;
    }

    [[nodiscard]]
    constexpr point2d_type p01() const noexcept
    {
        return { x0(), y1() };
    }

    [[nodiscard]]
    constexpr point2d_type p10() const noexcept
    {
        return { x1(), y0() };
    }

    [[nodiscard]]
    constexpr point2d_type p11() const noexcept
    {
        return { x1(), y1() };
    }

    [[nodiscard]]
    constexpr value_type x() const noexcept
    {
        return position.x();
    }

    [[nodiscard]]
    constexpr value_type y() const noexcept
    {
        return position.y();
    }

    [[nodiscard]]
    constexpr value_type x0() const noexcept
    {
        return position.x();
    }

    [[nodiscard]]
    constexpr value_type y0() const noexcept
    {
        return position.y();
    }

    [[nodiscard]]
    constexpr value_type x1() const noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
        return static_cast<value_type>(x0() + width());
        D_WARNING_POP;
    }

    [[nodiscard]]
    constexpr value_type y1() const noexcept
    {
        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
        return static_cast<value_type>(y0() + height());
        D_WARNING_POP;
    }

    [[nodiscard]]
    constexpr size_type width() const noexcept
    {
        return sizes.width();
    }

    [[nodiscard]]
    constexpr size_type height() const noexcept
    {
        return sizes.height();
    }

    [[nodiscard]]
    constexpr bool operator == (const rectangle&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const rectangle&) const noexcept = default;
};

template<class T, class U>
rectangle(point2d<T>, size2d<U>) -> rectangle<T, U>;


template<class T, class U>
[[nodiscard]] constexpr const rectangle<T, U>& as_rectangle(const rectangle<T, U>& r) noexcept
{
    return r;
}

template<class T, class U>
[[nodiscard]] constexpr rectangle<T, U>& as_rectangle(rectangle<T, U>& r) noexcept
{
    return r;
}

template<template<class, class> class Rc, class T, class U, class NearT, class NearU>
[[nodiscard]] constexpr auto md_round_to_near(const Rc<T, U>& v, const Rc<NearT, NearU>& v_near) noexcept -> Rc<
    decltype(as_point2d(md_round_to_near(as_rectangle(v).position, as_rectangle(v_near).position))._0),
    decltype(as_size2d(md_round_to_near(as_rectangle(v).sizes, as_rectangle(v_near).sizes))._0)
>
{
    return
    {
        .position{ md_round_to_near(v.position, v_near.position) },
        .sizes{ md_round_to_near(v.sizes, v_near.sizes) }
    };
}