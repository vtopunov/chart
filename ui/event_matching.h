#pragma once

#include <core/invoke.h>

#include <ui/event.h>


namespace ui
{
    constexpr auto invoke_event = ::invoke_if_exist_r_or_null<event_result_opt_t>;


    template<class T>
    constexpr event_result_opt_t do_event_match(T& p, const event& e) noexcept
    {
        switch (e.style())
        {
#if defined(D_OS_WINDOWS)
            case event_style::size:               return invoke_event(p, event_for<event_style::size>(e));
            case event_style::mouse_wheel:        return invoke_event(p, event_for<event_style::mouse_wheel>(e));
            case event_style::mouse_double_click: return invoke_event(p, event_for<event_style::mouse_double_click>(e));
#endif

            case event_style::mouse_move:         return invoke_event(p, event_for<event_style::mouse_move>(e));
            case event_style::mouse_down:         return invoke_event(p, event_for<event_style::mouse_down>(e));
            case event_style::mouse_up:           return invoke_event(p, event_for<event_style::mouse_up>(e));

            default:
                break;
        }

        return invoke_event(p, e);
    }

    struct nullcmdevent_result_t 
    {};

    constexpr nullcmdevent_result_t nullcmdevent_result{};

    constexpr auto invoke_cmd_event = ::invoke_if_exist_r_or_null<nullcmdevent_result_t>;


    template<class T>
    constexpr nullcmdevent_result_t do_cmd_event_match(T& p, const cmd_event& e) noexcept
    {
        switch (e.style())
        {
            case cmd_event_style::redraw_needed:
                return invoke_cmd_event(p, cmd_event_for<cmd_event_style::redraw_needed>(e));
            case cmd_event_style::content_rect_changed: 
                return invoke_cmd_event(p, cmd_event_for<cmd_event_style::content_rect_changed>(e));

            default:
                break;
        }

        return invoke_cmd_event(p, e);
    }


    template<class Processor>
    struct event_match
    {
        static_assert(!std::is_reference_v<Processor>);

        Processor processor;

        constexpr event_result_opt_t operator () (const event& e) noexcept
        {
            return do_event_match(::unrefwrap(processor), e);
        }
    };

    template<class T>
    event_match(T)->event_match<std::remove_reference_t<T>>;
}

