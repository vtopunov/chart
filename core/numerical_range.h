#pragma once

#include "vec.h"

template<class T>
struct numerical_range
{
    vec<T> bounds{};

    constexpr numerical_range() noexcept = default;

    explicit constexpr numerical_range(vec<T> vector_bounds) noexcept
        : bounds{ vector_bounds }
    {}

    constexpr numerical_range(T front_bound, T back_bound) noexcept
        : bounds{ front_bound, back_bound }
    {}

    template<size_t index>
    constexpr T get() const noexcept
    {
        return bounds.get<index>();
    }

    constexpr bool includes(T value) const noexcept
    {
        return value >= min() && value <= max();
    }

    constexpr bool includes(numerical_range range) const noexcept
    {
        return range.min() >= min() && range.max() <= max();
    }

    constexpr T front() const noexcept
    {
        return get<0>();
    }

    constexpr T back() const noexcept
    {
        return get<1>();
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

    constexpr numerical_range with_inverse() const noexcept
    {
        return numerical_range{ bounds.with_reverse() };
    }

    constexpr numerical_range with_inclusion(T value) const noexcept
    {
        return 
        { 
            numerical_range{ bounds._0, value }.min(),
            numerical_range{ value, bounds._1 }.max()
        };
    }

    constexpr numerical_range with_moving(T move) const noexcept
    {
        return numerical_range{ bounds + vec<T>::fill(move) };
    }
};

template<class T>
constexpr bool operator == ( numerical_range<T> left, numerical_range<T> right ) noexcept
{
    return left.bounds == right.bounds;
}

template<class T>
constexpr bool operator != ( numerical_range<T> left, numerical_range<T> right ) noexcept
{
    return !( left == right );
}