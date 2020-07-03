#pragma once

#include <core/vec.h>
#include <core/num_range.h>
#include <core/point.h>
#include <core/polynomial.h>

template<class To, class From>
constexpr decltype(auto) lerp(const vec<From>& from, const vec<To>& to) noexcept
{
    const auto difference_from = from._1 - from._0;
    D_ASSERT(difference_from);

    const auto scaling = (to._1 - to._0) / difference_from;
    const auto offset = (to._0 * from._1 - to._1 * from._0) / difference_from;

    return polynomial{ offset, scaling };
}

template<class To, class From>
constexpr decltype(auto) lerp(const num_range<From>& from, const num_range<To>& to) noexcept
{
    return lerp(from.bounds, to.bounds);
}

template<class T>
constexpr decltype(auto) lerp(const point<T>& p0, const point<T>& p1) noexcept
{
    return lerp
    (
        vec{ p0.x(), p1.x() },
        vec{ p0.y(), p1.y() }
    );
}