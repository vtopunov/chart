#pragma once

#include <core/num_range.h>
#include <core/point2d.h>
#include <core/polynomial.h>


template<class From, class To>
[[nodiscard]] constexpr decltype(auto) lerp(const num_range<From>& from, const num_range<To>& to) noexcept
{
    const auto x_length = from.length();
    D_ASSERT(x_length); // + D_ASSERT(isnormal(x)) for floating point, c++23 constexpr

    const auto scaling = to.length() / x_length;
    const auto offset = (to._0 * from._1 - to._1 * from._0) / x_length;

    return polynomial2{ offset, scaling };
}

template<class T>
[[nodiscard]] constexpr decltype(auto) lerp(const point2d<T>& p0, const point2d<T>& p1) noexcept
{
    return lerp
    (
        num_range{ p0.x(), p1.x() },
        num_range{ p0.y(), p1.y() }
    );
}

template<class T>
[[nodiscard]] constexpr decltype(auto) lerp(const num_range<point2d<T>>& line) noexcept
{
    return lerp(line._0, line._1);
}
