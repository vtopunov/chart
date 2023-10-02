#pragma once

#include <core/limits.h>
#include <core/underlying.h>

#include <ui/fwd.h>


namespace widget
{
    class window;

    template<class... Types>
    class common_context;

    template<class EventBase, class... Args>
    class widget_event;

    struct redraw_event_base {};

    constexpr redraw_event_base redraw_event_base_v{};

    template<class... Args>
    using redraw_event = widget_event<redraw_event_base, Args...>;

    template<class... Args>
    using mouse_move_event = widget_event<ui::mouse_move_event, Args...>;

    template<class... Args>
    using mouse_up_event = widget_event<ui::mouse_up_event, Args...>;

#ifdef D_OS_WINDOWS
    template<class... Args>
    using mouse_double_click_event = widget_event<ui::mouse_double_click_event, Args...>;
#endif


    enum class window_configation : size_t
    {
        nocfg,
        badcfg = numeric_max_v<size_t>
    };

    constexpr auto nocfg = window_configation::nocfg;
    constexpr auto badcfg = window_configation::badcfg;

    [[nodiscard]]
    constexpr window_configation operator | (window_configation left, window_configation right) noexcept
    {
        return e_bit_or(left, right);
    }

    [[nodiscard]]
    constexpr window_configation operator | (window_configation left, bool right) noexcept
    {
        return (right) ? left : badcfg;
    }

    [[nodiscard]]
    constexpr window_configation operator | (bool left, window_configation right) noexcept
    {
        return right | left;
    }


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
}