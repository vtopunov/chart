#pragma once

#include <core/num_range.h>
#include <core/point.h>

template<class T>
struct rect;

template<class T>
struct rect
{
    using value_type = T;
    using point_type = point<value_type>;
    using point_range_type = num_range<point_type>;
    using axis_range_type = num_range<value_type>;

    point_range_type diagonal;


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

    constexpr rect with_zooming( point_type zoom ) const noexcept
    {
        const auto center = diagonal.center();

        const point_type radius
        {
            ( zoom.x() * width() ) / 2,
            ( zoom.y() * height() ) / 2
        };

        return { center - radius, center + radius };
    }

    constexpr rect with_inclusion( point_type point ) const noexcept
    {
        return rect{ diagonal.with_inclusion( point ) };
    }

    constexpr rect with_inverse() const noexcept
    {
        return rect{ diagonal.with_inverse() };
    }

    constexpr rect with_moving( point_type move ) const noexcept
    {
        return rect{ diagonal.with_moving( move ) };
    }

    constexpr rect with_frame( value_type frame_width, value_type frame_height ) const noexcept
    {
        const auto point = make_point( frame_width, frame_height );
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
constexpr rect<T> make_rect( point<T> beginning_of_diagonal, point<T> end_of_diagonal ) noexcept
{
    return { make_num_range( beginning_of_diagonal, end_of_diagonal ) };
}

template<class T>
constexpr rect<T> make_rect( num_range<T> x, num_range<T> y ) noexcept
{
    return make_rect(
        make_point( x.bounds._0, y.bounds._0 ),
        make_point( x.bounds._1, y.bounds._1 )
    );
}

template<class T>
constexpr rect<T> make_rect( x_axis_type, num_range<T> x, num_range<T> y ) noexcept
{
    return make_rect( x, y );
}

template<class T>
constexpr rect<T> make_rect( y_axis_type, num_range<T> y, num_range<T> x ) noexcept
{
    return make_rect( x, y );
}

template<axis_type axis, class T>
constexpr rect<T> inverse_axis( rect<T> rect ) noexcept
{
    return make_rect(
        axis_constant<axis>(),
        inverse(rect.axis_range<axis>()),
        rect.axis_range<other_axis_v<axis>>()
    );
}

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