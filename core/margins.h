#pragma once

#include <core/point2d.h>


template<class T>
struct margins
{
    T left;
    T top;
    T right;
    T bottom;

    [[nodiscard]] constexpr point2d<T> left_top() const noexcept
    {
        return { left, top };
    }

    [[nodiscard]] constexpr point2d<T> right_bottom() const noexcept
    {
        return { right, bottom };
    }
};