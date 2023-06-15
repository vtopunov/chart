#pragma once

#include <chrono>

#include <ui/event.h>


namespace ui
{
    template<class T, event_style es>
    auto call_event(T& function, const specialized_event<es>& e) noexcept -> decltype(function(e))
    {
        return function(e);
    }

    template<class T>
    auto call_event(T& function, const event& e) noexcept -> decltype(function(e))
    {
        return function(e);
    }

    template<class T>
    constexpr std::nullopt_t call_event(T&, no_overload_for<event>) noexcept
    {
        return std::nullopt;
    }

    template<class T>
    event_result_opt_t do_event_match(T& p, const event& e) noexcept
    {
        switch (e.style())
        {
#if defined(D_OS_WINDOWS)
            case event_style::size:               return call_event(p, event_for<event_style::size>(e));
            case event_style::mouse_wheel:        return call_event(p, event_for<event_style::mouse_wheel>(e));
            case event_style::mouse_double_click: return call_event(p, event_for<event_style::mouse_double_click>(e));
#endif
            case event_style::mouse_move:         return call_event(p, event_for<event_style::mouse_move>(e));
            case event_style::mouse_down:         return call_event(p, event_for<event_style::mouse_down>(e));
            case event_style::mouse_up:           return call_event(p, event_for<event_style::mouse_up>(e));

            default:
                break;
        }

        return call_event(p, e);
    }

    template<class Processor>
    struct event_match
    {
        static_assert(!std::is_reference_v<Processor>);

        Processor processor;

        event_result_opt_t operator () (const event& e) noexcept
        {
            using processor_lvalue_reference_t 
                = std::add_lvalue_reference_t<std::remove_reference_t<remove_reference_wrapper_t<Processor>>>;

            return do_event_match(static_cast<processor_lvalue_reference_t>(processor), e);
        }
    };

    template<class T>
    event_match(T)->event_match<std::remove_reference_t<T>>;
}

