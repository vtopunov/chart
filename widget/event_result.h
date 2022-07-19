#pragma once

#include <core/underlying.h>


namespace widget
{
    enum class event_result : size_t
    {
        idle = 0,
        redraw = (1 << 0)
    };


    [[nodiscard]]
    constexpr event_result operator | (event_result left, event_result right) noexcept
    {
        return e_bit_or(left, right);
    }

    constexpr event_result& operator |= (event_result& left, event_result right) noexcept
    {
        return left = (left | right);
    }

    constexpr event_result& operator |= (event_result& left, std::nullopt_t) noexcept
    {
        return left;
    }


    [[nodiscard]]
    constexpr event_result operator & (event_result left, event_result right) noexcept
    {
        return e_bit_and(left, right);
    }

    constexpr event_result& operator &= (event_result& left, event_result right) noexcept
    {
        return left = (left & right);
    }


    [[nodiscard]]
    constexpr event_result operator ~ (event_result e) noexcept
    {
        return e_bit_not(e);
    }
}