#pragma once

#include <core/vec.h>

template<class T>
struct size2d : vec2<T>
{
    using vec2_type = vec2<T>;
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
        return vec2_type::_0;
    }

    [[nodiscard]]
    constexpr const_reference cref_height() const noexcept
    {
        return vec2_type::_1;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return is_positive(width()) && is_positive(height());
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
