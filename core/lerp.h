#pragma once

#include <core/vec.h>
#include <core/num_range.h>
#include <core/polynomial.h>

template<class To, class From> [[nodiscard]]
constexpr decltype(auto) lerp(const num_range<From>& from, const num_range<To>& to) noexcept
{
    const auto length_from = from.length();
    D_ASSERT(length_from);

    const auto scaling = to.length() / length_from;
    const auto offset = (to.front() * from.back() - to.back() * from.front()) / length_from;

    return polynomial2{ offset, scaling };
}


template<class T> [[nodiscard]]
constexpr decltype(auto) lerp(const num_range<vec2<T>>& line) noexcept
{
    return lerp
    (
        num_range{ line.bounds._0.x(), line.bounds._1.x() },
        num_range{ line.bounds._0.y(), line.bounds._1.y() }
    );
}