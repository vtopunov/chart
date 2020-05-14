#pragma once

#include <core/num_range.h>
#include <core/point.h>

template<class T>
struct rect;

template<class T>
struct rect_size
{
    point<T> measures;

    constexpr T width() const noexcept
    {
        return measures.x();
    }

    constexpr T height() const noexcept
    {
        return measures.y();
    }

    constexpr point<T> to_point() const noexcept
    {
        return measures;
    }
};

template<class T>
constexpr rect_size<T> make_rect_size(point<T> area) noexcept
{
    return { area };
}

template<class T>
constexpr rect_size<T> make_rect_size(vec<T> area) noexcept
{
    return make_rect_size(make_point(area));
}

template<class T>
constexpr rect_size<T> make_rect_size(T width, T height) noexcept
{
    return make_rect_size(make_point(width, height));
}

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

    constexpr point_type v00() const noexcept
    {
        return diagonal.front();
    }

    constexpr point_type v01() const noexcept
    {
        return { x0(), y1() };
    }

    constexpr point_type v10() const noexcept
    {
        return { x1(), y0() };
    }

    constexpr point_type v11() const noexcept
    {
        return diagonal.back();
    }

    constexpr rect_size<T> size() const noexcept
    {
        return make_rect_size(diagonal.length());
    }

    constexpr value_type x0() const noexcept
    {
        return diagonal.front().x();
    }

    constexpr value_type y0() const noexcept
    {
        return diagonal.front().y();
    }

    constexpr value_type x1() const noexcept
    {
        return diagonal.back().x();
    }

    constexpr value_type y1() const noexcept
    {
        return diagonal.back().y();
    }

    constexpr value_type width() const noexcept
    {
        return x_axis_range().length();
    }

    constexpr value_type height() const noexcept
    {
        return y_axis_range().length();
    }

    constexpr rect with_zooming(const point_type& zoom) const noexcept
    {
        const auto center = diagonal.center();

        const point_type radius
        {
            ( zoom.x() * width() ) / 2,
            ( zoom.y() * height() ) / 2
        };

        return { center - radius, center + radius };
    }

    constexpr rect with_inclusion(const point_type& point) const noexcept
    {
        return rect{ diagonal.with_inclusion(point) };
    }

    constexpr rect with_moving(const point_type& move) const noexcept
    {
        return rect{ diagonal.with_moving(move) };
    }

    constexpr rect with_frame(value_type frame_width, value_type frame_height) const noexcept
    {
        const auto point = make_point(frame_width, frame_height);
        return rect{ diagonal.front() - point, diagonal.back() + point };
    }

    constexpr bool includes(const point_type& point) const noexcept
    {
        return x_axis_range().includes(point.x()) 
            && y_axis_range().includes(point.y());
    }

    constexpr bool includes(const rect& rect) const noexcept
    {
        return includes(rect.diagonal.front()) 
            && includes(rect.diagonal.back());
    }

    constexpr bool operator == (const rect&) const noexcept = default;

    constexpr bool operator != (const rect&) const noexcept = default;
};

template<class T>
constexpr rect<T> make_rect(const point<T>& beginning_of_diagonal, const point<T>& end_of_diagonal) noexcept
{
    return { make_num_range(beginning_of_diagonal, end_of_diagonal) };
}

template<class T>
constexpr rect<T> make_range_rect(const num_range<T>& x, const num_range<T>& y) noexcept
{
    return make_rect
    (
        make_point(x.bounds._0, y.bounds._0),
        make_point(x.bounds._1, y.bounds._1)
    );
}

template<class T>
constexpr rect<T> make_range_rect(x_axis_type, const num_range<T>& x, const num_range<T>& y) noexcept
{
    return make_range_rect(x, y);
}

template<class T>
constexpr rect<T> make_range_rect(y_axis_type, const num_range<T>& y, const num_range<T>& x) noexcept
{
    return make_range_rect(x, y);
}

template<axis_type axis, class T>
constexpr rect<T> inverse_axis(const rect<T>& rect) noexcept
{
    return make_range_rect
    (
        axis_constant<axis>(),
        inverse(rect.axis_range<axis>()),
        rect.axis_range<other_axis_v<axis>>()
    );
}