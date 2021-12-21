#pragma once

#include <core/vec.h>

template<class T>
struct point2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using reference = T&;
    using const_reference = const T&;

    [[nodiscard]]
    constexpr T x() const noexcept
    {
        return cref_x();
    }

    [[nodiscard]]
    constexpr T y() const noexcept
    {
        return cref_y();
    }

    [[nodiscard]]
    constexpr reference ref_x() noexcept
    {
        return as_mutable(cref_x());
    }

    [[nodiscard]]
    constexpr reference ref_y() noexcept
    {
        return as_mutable(cref_y());
    }

    [[nodiscard]]
    constexpr const_reference ref_x() const noexcept
    {
        return cref_x();
    }

    [[nodiscard]]
    constexpr const_reference ref_y() const noexcept
    {
        return cref_y();
    }

    [[nodiscard]]
    constexpr const_reference cref_x() const noexcept
    {
        return vec2_type::_0;
    }

    [[nodiscard]]
    constexpr const_reference cref_y() const noexcept
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


template<class T> [[nodiscard]]
constexpr decltype(auto) operator - (const point2d<T>& left, const point2d<T>& right) noexcept
{
    return point2d{ as_vec2(left) - as_vec2(right) };
}

template<class T> [[nodiscard]]
constexpr decltype(auto) operator + (const point2d<T>& left, const point2d<T>& right) noexcept
{
    return point2d{ as_vec2(left) + as_vec2(right) };
}

