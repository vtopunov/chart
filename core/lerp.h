#pragma once

#include <core/point2d.h>
#include <core/num_range.h>
#include <core/polynomial.h>

template<class To, class From> [[nodiscard]]
constexpr decltype(auto) lerp(const num_range<From>& from, const num_range<To>& to) noexcept
{
    const auto length_from = from.length();
    D_ASSERT(length_from);

    const auto scaling = to.length() / length_from;
    const auto offset = (to._0 * from._1 - to._1 * from._0) / length_from;

    return polynomial2{ offset, scaling };
}


template<class T> [[nodiscard]]
constexpr decltype(auto) lerp(const num_range<point2d<T>>& line) noexcept
{
    return lerp
    (
        num_range{ line._0.x(), line._1.x() },
        num_range{ line._0.y(), line._1.y() }
    );
}