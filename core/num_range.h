#pragma once

#include <core/vec2.h>

template<class T>
struct num_range : vec2<T>
{
    using vec2_type = vec2<T>;

    [[nodiscard]]
    constexpr decltype(auto) length() const noexcept
    {
        return difference(static_cast<const vec2_type&>(*this));
    }
};

template<class T>
num_range(T, T)->num_range<T>;

template<class T> [[nodiscard]]
constexpr num_range<T> inverse(const num_range<T>& range) noexcept
{
    return { reverse(range) };
}