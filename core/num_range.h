#pragma once

#include <core/vec2.h>


template<class T>
struct num_range : vec2<T>
{
    using vec2_type = vec2<T>;
    using vec2_type::_0;
    using vec2_type::_1;

    [[nodiscard]]
    constexpr decltype(auto) length() const noexcept
    {
        return _1 - _0;
    }
};

template<class T>
num_range(T, T)->num_range<T>;
