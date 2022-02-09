#pragma once

#include <core/type_traits.h>
#include <core/num_range.h>
#include <core/point2d.h>
#include <core/size2d.h>


template<class T> [[nodiscard]]
constexpr decltype(auto) operator + (const point2d<T>& right, const size2d<T>& left) noexcept
{
    return point2d{ as_vec2(left) + as_vec2(right) };
}

template<class T> [[nodiscard]]
constexpr decltype(auto) operator + (const size2d<T>& right, const point2d<T>& left) noexcept
{
    return size2d{ as_vec2(left) + as_vec2(right) };
}

template<class T> [[nodiscard]]
constexpr decltype(auto) operator - (const point2d<T>& left, const size2d<T>& right) noexcept
{
    return point2d{ as_vec2(left) - as_vec2(right) };
}

template<class T> [[nodiscard]]
constexpr decltype(auto) operator - (const size2d<T>& left, const point2d<T>& right) noexcept
{
    return size2d{ as_vec2(left) - as_vec2(right) };
}


namespace private_detail_rect
{
    template<class T>
    using make_unsigned_opt_t = conditional_op_t<std::is_integral_v<T>, std::make_unsigned_t, T>;
}


template<class T>
struct rect
{
    using scalar_type = T;
    using size_type = private_detail_rect::make_unsigned_opt_t<scalar_type>;
    using size2d_type = size2d<size_type>;
    using point2d_type = point2d<scalar_type>;
    using diagonal_line_type = num_range<point2d_type>;

    diagonal_line_type diagonal;

    [[nodiscard]]
    constexpr point2d_type p00() const noexcept
    {
        return diagonal._0;
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
        return diagonal._1;
    }

    [[nodiscard]]
    constexpr scalar_type x0() const noexcept
    {
        return diagonal._0.x();
    }

    [[nodiscard]]
    constexpr scalar_type y0() const noexcept
    {
        return diagonal._0.y();
    }

    [[nodiscard]]
    constexpr scalar_type x1() const noexcept
    {
        return diagonal._1.x();
    }

    [[nodiscard]]
    constexpr scalar_type y1() const noexcept
    {
        return diagonal._1.y();
    }

    [[nodiscard]]
    constexpr size2d_type sizes() const noexcept
    {
        return { width(), height() };
    }

    [[nodiscard]]
    constexpr size_type width() const noexcept
    {
        return narrow_cast<size_type>(x1() - x0());
    }

    [[nodiscard]]
    constexpr size_type height() const noexcept
    {
        return narrow_cast<size_type>(y1() - y0());
    }
};

template<class T>
rect(point2d<T>, point2d<T>)->rect<T>;