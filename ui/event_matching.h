#pragma once

#include <chrono>

#include <core/ordered_overload.h>

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


    template<class Processor>
    struct event_callback_instance
    {
        static event_result_opt_t callback(void* data, const event& e) noexcept
        {
            D_ASSERT(data);
            auto& processor = *static_cast<std::remove_reference_t<Processor>*>(data);

            switch (e.style())
            {
#if defined(D_OS_WINDOWS)
                case event_style::size:               return call_event(processor, event_specializing_for<event_style::size>(e));
                case event_style::mouse_wheel:        return call_event(processor, event_specializing_for<event_style::mouse_wheel>(e));
                case event_style::mouse_double_click: return call_event(processor, event_specializing_for<event_style::mouse_double_click>(e));
#endif
                case event_style::mouse_move:         return call_event(processor, event_specializing_for<event_style::mouse_move>(e));
                case event_style::mouse_down:         return call_event(processor, event_specializing_for<event_style::mouse_down>(e));
                case event_style::mouse_up:           return call_event(processor, event_specializing_for<event_style::mouse_up>(e));

                default: 
                    break;
            }

            return call_event(processor, e);
        }
    };
}

