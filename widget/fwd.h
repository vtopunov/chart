#pragma once

#include <ui/fwd.h>

#include <egl_ui/viewport_size2d.h>


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

    template<class... Args>
    using mouse_double_click_event = widget_event<ui::mouse_double_click_event, Args...>;


    enum class event_result : size_t
    {
        idle = 0,
        redraw = (1 << 0),
        invalid = numeric_max_v<size_t>
    };

    [[nodiscard]]
    constexpr event_result operator | (event_result left, event_result right) noexcept
    {
        return e_bit_or(left, right);
    }

    [[nodiscard]]
    constexpr event_result operator | (event_result left, bool right) noexcept
    {
        return (right) ? left : e_bit_or(left, event_result::invalid);
    }

    [[nodiscard]]
    constexpr event_result operator | (bool left, event_result right) noexcept
    {
        return right | left;
    }

    [[nodiscard]]
    constexpr bool event_result_is_invalid(event_result result) noexcept
    {
        return e_bit_check(result, event_result::invalid);
    }
}