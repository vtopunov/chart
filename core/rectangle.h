#pragma once

#include <core/point2d.h>
#include <core/size2d.h>


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
};

template<class T, class U>
rectangle(point2d<T>, size2d<U>)->rectangle<T, U>;


template<class T, class U> [[nodiscard]]
constexpr typename rectangle<T, U>::size2d_type sizes(const rectangle<T, U>& r) noexcept
{
    return r.sizes;
}

template<class T, class U> [[nodiscard]]
constexpr typename rectangle<T, U>::size_type width(const rectangle<T, U>& r) noexcept
{
    return r.width();
}

template<class T, class U> [[nodiscard]]
constexpr typename rectangle<T, U>::size_type height(const rectangle<T, U>& r) noexcept
{
    return r.height();
}
