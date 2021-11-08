#pragma once

#include <core/vec.h>

template<class T>
struct size2d : vec2<T>
{
    using vec2_type = vec2<T>;

    constexpr T width() const noexcept
    {
        return vec2_type::_0;
    }

    constexpr T height() const noexcept
    {
        return vec2_type::_1;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        constexpr T zero{};

        return ( width() > zero ) && ( height() > zero );
    }

    [[nodiscard]]
    constexpr bool operator == (const size2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const size2d&) const noexcept = default;
};

template<class T>
size2d(T, T)->size2d<T>;

template<class T>
size2d(const vec2<T>&)->size2d<T>;

using size2d_t = size2d<upixel_t>;
