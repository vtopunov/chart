#pragma once

#include <core/rectangle.h>
#include <core/lerp.h>


template<class A1, class A0 = A1>
struct transformation
{
    using function_type = polynomial2<A1, A0>;

    function_type fx;
    function_type fy;

    template<class T>
    [[nodiscard]] constexpr auto md_apply_fx(const T& v) const noexcept -> decltype(fx(v))
    {
        return fx(v);
    }

    template<template<class> class Vec, class T>
    [[nodiscard]] constexpr auto md_apply_fx(const Vec<T>& v) const noexcept
        -> Vec<decltype(md_apply_fx(as_vec2(v)._0))>
    {
        return
        {
            md_apply_fx(v._0),
            md_apply_fx(v._1)
        };
    }

    template<class T>
    [[nodiscard]] constexpr auto md_apply_fy(const T& v) const noexcept -> decltype(fy(v))
    {
        return fy(v);
    }

    template<template<class> class Vec, class T>
    [[nodiscard]] constexpr auto md_apply_fy(const Vec<T>& v) const noexcept
        -> Vec<decltype(md_apply_fy(as_vec2(v)._0))>
    {
        return
        {
            md_apply_fy(v._1)
        };
    }

    template<class T>
    [[nodiscard]] constexpr auto md_apply_fx_scale(const T& v) const noexcept -> decltype(fx.a1* v)
    {
        return fx.a1 * v;
    }

    template<template<class> class Vec, class T>
    [[nodiscard]] constexpr auto md_apply_fx_scale(const Vec<T>& v) const noexcept
        -> Vec<decltype(md_apply_fx_scale(as_vec2(v)._0))>
    {
        return
        {
            md_apply_fx_scale(v._0),
            md_apply_fx_scale(v._1)
        };
    }

    template<class T>
    [[nodiscard]] constexpr auto md_apply_fy_scale(const T& v) const noexcept -> decltype(fy.a1* v)
    {
        return fy.a1 * v;
    }

    template<template<class> class Vec, class T>
    [[nodiscard]] constexpr auto md_apply_fy_scale(const Vec<T>& v) const noexcept
        -> Vec<decltype(md_apply_fy_scale(as_vec2(v)._0))>
    {
        return
        {
            md_apply_fy_scale(v._0),
            md_apply_fy_scale(v._1)
        };
    }


    template<template<class> class Size2d, class T>
    constexpr auto operator () (const Size2d<T>& v) const noexcept
        -> Size2d<decltype(md_apply_fx_scale(as_size2d(v)._0))>
    {
        return
        {
            md_apply_fx_scale(v._0),
            md_apply_fy_scale(v._1)
        };
    }

    template<template<class> class Point2d, class T>
    constexpr auto operator () (const Point2d<T>& v) const noexcept
        -> Point2d<decltype(md_apply_fx(as_point2d(v)._0))>
    {
        return
        {
            md_apply_fx(v._0),
            md_apply_fy(v._1)
        };
    }

    template<template<class> class Vec, class T>
    constexpr auto operator () (const Vec<T>& v) const noexcept
        -> Vec<decltype((*this)(as_vec2(v)._0))>
    {
        return
        {
            (*this)(v._0),
            (*this)(v._1)
        };
    }

    template<template<class, class> class Rc, class T, class U>
    constexpr auto operator () (const Rc<T, U>& v) const noexcept -> Rc<
        decltype(as_point2d((*this)(as_rectangle(v).position))._0),
        decltype(as_size2d((*this)(as_rectangle(v).sizes))._0)
    >
    {
        return
        {
            (*this)(v.position),
            (*this)(v.sizes)
        };
    }

    [[nodiscard]]
    constexpr point2d<A0> shift() const noexcept
    {
        return { fx.a0, fy.a0 };
    }

    [[nodiscard]]
    constexpr size2d<A1> scale() const noexcept
    {
        return { fx.a1, fy.a1 };
    }

    [[nodiscard]]
    constexpr bool operator == (const transformation&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const transformation&) const noexcept = default;
};

template<class A1, class A0>
transformation(polynomial2<A1, A0>, polynomial2<A1, A0>) -> transformation<A1, A0>;

template<class T0, class T, class Lerp>
[[nodiscard]] constexpr auto make_transformation_fx(const vec2<point2d<T0>>& sys0, const vec2<point2d<T>>& sys, Lerp lerp) noexcept
{
    return lerp(sys0._0._0, sys0._1._0, sys._0._0, sys._1._0);
}

template<class T0, class T, class Lerp>
[[nodiscard]] constexpr auto make_transformation_fy(const vec2<point2d<T0>>& sys0, const vec2<point2d<T>>& sys, Lerp lerp) noexcept
{
    return lerp(sys0._0._1, sys0._1._1, sys._0._1, sys._1._1);
}

template<class T0, class T>
[[nodiscard]] constexpr auto make_transformation_fx(const vec2<point2d<T0>>& sys0, const vec2<point2d<T>>& sys) noexcept
{
    return make_transformation_fx(sys0, sys, lerp);
}

template<class T0, class T>
[[nodiscard]] constexpr auto make_transformation_fy(const vec2<point2d<T0>>& sys0, const vec2<point2d<T>>& sys) noexcept
{
    return make_transformation_fy(sys0, sys, lerp);
}


template<class T0, class T>
[[nodiscard]] constexpr auto make_transformation(const vec2<point2d<T0>>& sys0, const vec2<point2d<T>>& sys) noexcept
{
    return transformation
    {
        make_transformation_fx(sys0, sys),
        make_transformation_fy(sys0, sys)
    };
}

template<class T0, class T>
[[nodiscard]] constexpr auto make_scale_transformation(const vec2<point2d<T0>>& sys0, const vec2<point2d<T>>& sys) noexcept
{
    return transformation
    {
        make_transformation_fx(sys0, sys, lerp_scale),
        make_transformation_fy(sys0, sys, lerp_scale)
    };
}
