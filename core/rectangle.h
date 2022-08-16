#pragma once

#include <core/point2d.h>
#include <core/size2d.h>


template<class T>
[[nodiscard]] constexpr decltype(auto) operator + (const point2d<T>& right, const size2d<T>& left) noexcept
{
    return point2d{ as_vec2(left) + as_vec2(right) };
}

template<class T>
[[nodiscard]] constexpr decltype(auto) operator + (const size2d<T>& right, const point2d<T>& left) noexcept
{
    return size2d{ as_vec2(left) + as_vec2(right) };
}

template<class T>
[[nodiscard]] constexpr decltype(auto) operator - (const point2d<T>& left, const size2d<T>& right) noexcept
{
    return point2d{ as_vec2(left) - as_vec2(right) };
}

template<class T>
[[nodiscard]] constexpr decltype(auto) operator - (const size2d<T>& left, const point2d<T>& right) noexcept
{
    return size2d{ as_vec2(left) - as_vec2(right) };
}


template<class T>
struct rectangle
{
    using value_type = T;
    using size_type = unsigned_or_t<value_type>;
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
        return narrow_cast<value_type>(x0() + width());
    }

    [[nodiscard]]
    constexpr value_type y1() const noexcept
    {
        return narrow_cast<value_type>(y0() + height());
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

    template<class Point>
    [[nodiscard]] constexpr bool contains(const Point& p) const noexcept
    {
        using position_on_axis_t = std::decay_t<decltype(p.x())>;
        static_assert(std::is_same_v<position_on_axis_t, std::decay_t<decltype(p.y())>>);

        constexpr auto contains1d = [] (position_on_axis_t p, value_type p0, size_type dp) noexcept
        {
            return p >= p0 && p < (p0 + dp);
        };

        return contains1d(p.x(), position.x(), sizes.width())
            && contains1d(p.y(), position.y(), sizes.height());
    }
};

template<class T>
rectangle(point2d<T>, size2d<unsigned_or_t<T>>)->rectangle<T>;


template<class T>
constexpr typename rectangle<T>::size2d_type sizes(const rectangle<T>& r) noexcept
{
    return r.sizes;
}

template<class T>
constexpr typename rectangle<T>::size_type width(const rectangle<T>& r) noexcept
{
    return r.width();
}

template<class T>
constexpr typename rectangle<T>::size_type height(const rectangle<T>& r) noexcept
{
    return r.height();
}
