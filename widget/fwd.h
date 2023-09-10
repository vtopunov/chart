#pragma once

#include <core/limits.h>
#include <core/underlying.h>

#include <ui/fwd.h>


namespace widget
{
    struct window;

    enum class window_configation : size_t 
    {
        nocfg,
        badcfg = numeric_max_v<size_t>
    };

    constexpr auto nocfg = window_configation::nocfg;
    constexpr auto badcfg = window_configation::badcfg;

    constexpr window_configation operator | (window_configation left, window_configation right) noexcept
    {
        return e_bit_or(left, right);
    }

    constexpr window_configation& operator |= (window_configation& left, window_configation right) noexcept
    {
        return e_bit_or_eq(left, right);
    }

    constexpr window_configation& operator |= (window_configation& left, bool right) noexcept
    {
        if (!right) [[unlikely]]
        {
            left = badcfg;
        }

        return left;
    }


    enum class event_result : size_t
    {
        idle = 0,
        redraw = (1 << 0)
    };

    constexpr event_result& operator |= (event_result& left, event_result right) noexcept
    {
        return e_bit_or_eq(left, right);
    }

    constexpr event_result& operator |= (event_result& left, bool right) noexcept
    {
        const auto e_right = (right) ? event_result::redraw : event_result::idle;
        return left |= e_right;
    }
}