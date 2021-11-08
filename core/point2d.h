#pragma once


#include <core/vec.h>

template<class T>
struct point2d : vec2<T>
{
    using vec2_type = vec2<T>;

    constexpr T x() const noexcept
    {
        return vec2_type::_0;
    }

    constexpr T y() const noexcept
    {
        return vec2_type::_1;
    }

    [[nodiscard]]
    constexpr bool operator == (const point2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const point2d&) const noexcept = default;
};

template<class T>
point2d(T, T)->point2d<T>;

template<class T>
point2d(const vec2<T>&)->point2d<T>;

using point2d_px_t = point2d<pixel_t>;
