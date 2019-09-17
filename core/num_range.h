#pragma once

#include "vec.h"

template<class T>
struct num_range
{
    vec<T> bounds;

    template<size_t index>
    constexpr T get() const noexcept
    {
        return bounds.get<index>();
    }

    constexpr bool includes(T value) const noexcept
    {
        return value >= min() && value <= max();
    }

    constexpr bool includes(num_range range) const noexcept
    {
        return range.min() >= min() && range.max() <= max();
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
        return bounds.difference();
    }

    constexpr T center() const noexcept
    {
        return bounds.mean();
    }

    constexpr T max() const noexcept
    {
        return bounds.max();
    }

    constexpr T min() const noexcept
    {
        return bounds.min();
    }

    constexpr num_range with_inclusion(T value) const noexcept
    {
        return 
        { 
            num_range{ { bounds._0, value } }.min(),
            num_range{ { value, bounds._1 } }.max()
        };
    }

    constexpr num_range with_moving(T move) const noexcept
    {
        return num_range{ bounds + fill_vec( move ) };
    }
};

template<class T>
constexpr num_range<T> make_num_range( T front, T back ) noexcept
{
    return { { front, back } };
}

template<class T>
constexpr num_range<T> inverse( num_range<T> range ) noexcept
{
    return { reverse( range.bounds ) };
}

template<class T>
constexpr bool operator == ( num_range<T> left, num_range<T> right ) noexcept
{
    return left.bounds == right.bounds;
}

template<class T>
constexpr bool operator != ( num_range<T> left, num_range<T> right ) noexcept
{
    return !( left == right );
}