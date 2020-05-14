#pragma once

#include <core/assert.h>
#include <core/vec.h>

template<class T>
struct num_range
{
    vec<T> bounds;

    template<size_t index>
    constexpr T get() const noexcept
    {
        return bounds.get<index>();
    }

    constexpr T front() const noexcept
    {
        return bounds._0;
    }

    constexpr T back() const noexcept
    {
        return bounds._1;
    }

    constexpr T length() const noexcept
    {
        return difference(bounds);
    }

    constexpr T center() const noexcept
    {
        return mean(bounds);
    }

    constexpr bool includes(const T& value) const noexcept
    {
        if constexpr ( !std::is_arithmetic_v<T> )
        {
            D_ASSERT(!"invalid type");
            return false;
        }

        return value >= min(*this) && value <= max(*this);
    }

    constexpr num_range with_inclusion(const T& value) const noexcept
    {
        return 
        { 
            min(make_vec(bounds._0, value)),
            max(make_vec(value, bounds._1))
        };
    }

    constexpr num_range with_moving(const T& move) const noexcept
    {
        return num_range{ bounds + fill_vec( move ) };
    }

    constexpr bool operator == (const num_range&) const noexcept = default;

    constexpr bool operator != (const num_range&) const noexcept = default;
};

template<class T>
constexpr T min(const num_range<T>& range) noexcept
{
    return min(range.bounds);
}

template<class T>
constexpr T max(const num_range<T>& range) noexcept
{
    return max(range.bounds);
}


template<class T>
constexpr num_range<T> make_num_range( const T& front, const T& back ) noexcept
{
    return { { front, back } };
}

template<class T>
constexpr num_range<T> inverse( const num_range<T>& range ) noexcept
{
    return { reverse( range.bounds ) };
}