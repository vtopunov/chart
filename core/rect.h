#pragma once

#include "numerical_range.h"
#include "point.h"

template<class T>
struct rect
{
    using value_type = T;
    using point_type = point<value_type>;
    using point_range_type = numerical_range<point_type>;
    using axis_range_type = numerical_range<value_type>;

    point_range_type diagonal{};

    constexpr rect() noexcept = default;

    explicit constexpr rect(point_range_type diagonal) noexcept
        : diagonal{ diagonal }
    {}

    constexpr rect(point_type beginning_of_diagonal, point_type end_of_diagonal) noexcept
        : diagonal{ beginning_of_diagonal, end_of_diagonal }
    {}

    constexpr rect(axis_range_type x, axis_range_type y) noexcept
        : diagonal
        { 
            point_type{ x.get<0>(), y.get<0>() },
            point_type{ x.get<1>(), y.get<1>() }
        }
    {}

    constexpr rect(x_axis_type, axis_range_type first, axis_range_type second) noexcept
        : rect{ first, second }
    {}

    constexpr rect(y_axis_type, axis_range_type first, axis_range_type second) noexcept
        : rect{ second, first }
    {}

    template<axis_type axis>
    constexpr axis_range_type axis_range() const noexcept
    {
        return 
        {
            diagonal.front().get<axis>(),
            diagonal.back().get<axis>()
        };
    }

    constexpr axis_range_type x_axis_range() const noexcept
    {
        return axis_range<axis_type::X>();
    }

    constexpr axis_range_type y_axis_range() const noexcept
    {
        return axis_range<axis_type::Y>();
    }

    constexpr point_type size() const noexcept
    {
        return diagonal.length();
    }

    constexpr value_type x() const noexcept
    {
        return diagonal.front().x();
    }

    constexpr value_type y() const noexcept
    {
        return diagonal.front().y();
    }

    constexpr value_type width() const noexcept
    {
        return x_axis_range().length();
    }

    constexpr value_type height() const noexcept
    {
        return y_axis_range().length();
    }

    template<axis_type axis>
    constexpr rect with_inverse_axis() const noexcept
    {
        return
        {
            axis_constant<axis>(),
            axis_range<axis>().with_inverse(),
            axis_range<other_axis_v<axis>>()
        };
    }

    constexpr rect with_zooming(point_type zoom) const noexcept
    {
        const auto center = diagonal.center();

        const point_type radius
        {
            ( zoom.x() * width() ) / 2,
            ( zoom.y() * height() ) / 2
        };

        return { center - radius, center + radius };
    }

    constexpr rect with_inclusion(point_type point) const noexcept
    {
        return rect{ diagonal.with_inclusion( point ) };
    }

    constexpr rect with_inverse() const noexcept
    {
        return rect{ diagonal.with_inverse() };
    }

    constexpr rect with_moving(point_type move) const noexcept
    {
        return rect{ diagonal.with_moving( move ) };
    }

    constexpr rect with_frame(value_type frame_width) const noexcept
    {
        const auto point = point_type::fill( frame_width );
        return rect{ diagonal.front() - point, diagonal.back() + point };
    }

    constexpr bool includes( point_type point ) const noexcept
    {
        return diagonal.includes( point );
    }

    constexpr bool includes( rect rect ) const noexcept
    {
        return diagonal.includes( rect.diagonal );
    }
};

template<class T>
constexpr bool operator == ( rect<T> left, rect<T> right ) noexcept
{
    return left.diagonal == right.diagonal;
}

template<class T>
constexpr bool operator != ( rect<T> left, rect<T> right ) noexcept
{
    return !( left == right );
}

using rect_t = rect<real_t>;