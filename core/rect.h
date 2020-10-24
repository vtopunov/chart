#pragma once

#include <core/num_range.h>

template<class T>
struct rect;

template<class T>
struct rect
{
    using vec_t = vec2<T>;
    using diagonal_t = num_range<vec_t>;

    diagonal_t diagonal;

    [[nodiscard]]
    constexpr vec_t v00() const noexcept
    {
        return diagonal.front();
    }

    [[nodiscard]]
    constexpr vec_t v01() const noexcept
    {
        return { x0(), y1() };
    }

    [[nodiscard]]
    constexpr vec_t v10() const noexcept
    {
        return { x1(), y0() };
    }

    [[nodiscard]]
    constexpr vec_t v11() const noexcept
    {
        return diagonal.back();
    }

    [[nodiscard]]
    constexpr vec_t center() const noexcept
    {
        return diagonal.center();
    }

    [[nodiscard]]
    constexpr T x0() const noexcept
    {
        return diagonal.front().x();
    }

    [[nodiscard]]
    constexpr T y0() const noexcept
    {
        return diagonal.front().y();
    }

    [[nodiscard]]
    constexpr T x1() const noexcept
    {
        return diagonal.back().x();
    }

    [[nodiscard]]
    constexpr T y1() const noexcept
    {
        return diagonal.back().y();
    }

    [[nodiscard]]
    constexpr vec_t sizes() const noexcept
    {
        return diagonal.length();
    }

    [[nodiscard]]
    constexpr T width() const noexcept
    {
        return x1() - x0();
    }

    [[nodiscard]]
    constexpr T height() const noexcept
    {
        return y1() - y0();
    }

    [[nodiscard]]
    constexpr rect with_zooming(vec_t zoom_sizes) const noexcept
    {
        const auto new_radius = ( zoom_sizes * sizes() ) / 2;
        const auto fix_center = center();
        return { fix_center - new_radius, fix_center + new_radius };
    }

    [[nodiscard]]
    constexpr rect with_inclusion(vec_t point) const noexcept
    {
        return rect{ diagonal.with_inclusion(point) };
    }

    [[nodiscard]]
    constexpr rect with_moving(vec_t move) const noexcept
    {
        return rect{ diagonal.with_moving(move) };
    }

    [[nodiscard]]
    constexpr rect with_frame(T width) const noexcept
    {
        const vec_t radius_inc{ width, width };
        return rect{ diagonal.front() - radius_inc, diagonal.back() + radius_inc };
    }

    [[nodiscard]]
    constexpr bool operator == (const rect&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const rect&) const noexcept = default;
};

template<class T>
rect(vec2<T>, vec2<T>)->rect<T>;

enum class axis : size_t
{
    x,
    y
};

template<axis a, class T> [[nodiscard]]
constexpr rect<T> inverse_axis(const rect<T>& rect) noexcept
{
    if constexpr ( a == axis::x )
    {
        return
        {
            rect.v10(),
            rect.v01(),
        };
    }
    else
    {
        return
        {
            rect.v01(),
            rect.v10()
        };
    }
}