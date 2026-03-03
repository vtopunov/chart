#pragma once

#include <core/vec2.h>


template<class T>
struct point2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using typename vec2_type::value_type;
    using typename vec2_type::reference;
    using typename vec2_type::const_reference;
    using vec2_type::_0;
    using vec2_type::_1;


    [[nodiscard]]
    constexpr value_type x() const noexcept
    {
        return cref_x();
    }

    [[nodiscard]]
    constexpr value_type y() const noexcept
    {
        return cref_y();
    }

    [[nodiscard]]
    constexpr point2d with_x(value_type x) const noexcept
    {
        return { std::move(x), _1 };
    }

    [[nodiscard]]
    constexpr point2d with_y(value_type y) const noexcept
    {
        return { _0, std::move(y) };
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
        return _0;
    }

    [[nodiscard]]
    constexpr const_reference cref_y() const noexcept
    {
        return _1;
    }

    [[nodiscard]]
    constexpr bool operator == (const point2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const point2d&) const noexcept = default;
};

template<class T>
point2d(T, T) -> point2d<T>;

template<class T>
point2d(const vec2<T>&) -> point2d<T>;


template<class T>
[[nodiscard]] constexpr const point2d<T>& as_point2d(const point2d<T>& p) noexcept
{
    return p;
}

template<class T>
[[nodiscard]] constexpr point2d<T>& as_point2d(point2d<T>& p) noexcept
{
    return p;
}


template<class T>
[[nodiscard]] constexpr point2d<T> to_point2d(const vec2<T>& v) noexcept
{
    return { v };
}