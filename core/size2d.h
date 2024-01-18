#pragma once

#include <core/vec2.h>


template<class T>
struct size2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using vec2_type::_0;
    using vec2_type::_1;
    using reference = T&;
    using const_reference = const T&;

    [[nodiscard]]
    constexpr T width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr T height() const noexcept
    {
        return cref_height();
    }

    [[nodiscard]]
    constexpr reference ref_width() noexcept
    {
        return as_mutable(cref_width());
    }

    [[nodiscard]]
    constexpr reference ref_height() noexcept
    {
        return as_mutable(cref_height());
    }

    [[nodiscard]]
    constexpr const_reference ref_width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr const_reference ref_height() const noexcept
    {
        return cref_height();
    }

    [[nodiscard]]
    constexpr const_reference cref_width() const noexcept
    {
        return _0;
    }

    [[nodiscard]]
    constexpr const_reference cref_height() const noexcept
    {
        return _1;
    }

    [[nodiscard]]
    constexpr size2d with_width(T value) const noexcept
    {
        return { std::move(value), _1 };
    }

    [[nodiscard]]
    constexpr size2d with_height(T value) const noexcept
    {
        return { _0, std::move(value) };
    }

    [[nodiscard]]
    constexpr bool operator == (const size2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const size2d&) const noexcept = default;
};

template<class T>
size2d(T, T) -> size2d<T>;

template<class T>
size2d(const vec2<T>&) -> size2d<T>;


template<class T>
[[nodiscard]] constexpr const size2d<T>& as_size2d(const size2d<T>& sizes) noexcept
{
    return sizes;
}

template<class T>
[[nodiscard]] constexpr size2d<T>& as_size2d(size2d<T>& sizes) noexcept
{
    return sizes;
}

template<class T>
[[nodiscard]] constexpr size2d<T> to_size2d(const vec2<T>& v) noexcept
{
    return { v }; 
}

template<class T>
[[nodiscard]] constexpr T width(const size2d<T>& sizes) noexcept
{
    return sizes.width();
}

template<class T>
[[nodiscard]] constexpr T height(const size2d<T>& sizes) noexcept
{
    return sizes.height();
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator * (const Vec<L>& left, const Vec<R>& right) noexcept -> Vec<decltype(as_size2d(left)._0 * as_size2d(right)._0)>
{
    return
    { 
        left._0 * right._0, 
        left._1 * right._1
    };
}

template<template<class> class Vec, class L, class R>
[[nodiscard]] constexpr auto operator / (const Vec<L>& left, const Vec<R>& right) noexcept -> Vec<decltype(as_size2d(left)._0 / as_size2d(right)._0)>
{
    return
    { 
        left._0 / right._0, 
        left._1 / right._1
    };
}