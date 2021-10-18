#pragma once

#include <core/assert.h>
#include <core/vec.h>

template<class T>
struct num_range
{
    vec2<T> bounds;

    [[nodiscard]]
    constexpr T front() const noexcept
    {
        return bounds._0;
    }

    [[nodiscard]]
    constexpr T back() const noexcept
    {
        return bounds._1;
    }

    [[nodiscard]]
    constexpr decltype(auto) length() const noexcept
    {
        return difference(bounds);
    }

    [[nodiscard]]
    constexpr decltype(auto) center() const noexcept
    {
        return mean(bounds);
    }

    [[nodiscard]]
    constexpr bool includes(const T& value) const noexcept
    {
        static_assert( std::is_arithmetic_v<T> );

        const auto sorted = (bounds._1 < bounds._0) ? reverse(bounds) : bounds;

        return ( bounds._0 <= value ) && ( value <= bounds._1 );
    }

    [[nodiscard]]
    constexpr num_range with_inclusion(const T& value) const noexcept
    {
        return
        {
            min(vec2{ bounds._0, value }),
            max(vec2{ value, bounds._1 })
        };
    }

    [[nodiscard]]
    constexpr num_range with_moving(const T& move) const noexcept
    {
        return num_range{ bounds + fill_vec2(move) };
    }

    [[nodiscard]]
    constexpr bool operator == (const num_range&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const num_range&) const noexcept = default;
};

template<class T>
num_range(T, T)->num_range<T>;

template<class T> [[nodiscard]]
constexpr T min(const num_range<T>& range) noexcept
{
    return min(range.bounds);
}

template<class T> [[nodiscard]]
constexpr T max(const num_range<T>& range) noexcept
{
    return max(range.bounds);
}