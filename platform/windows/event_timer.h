#pragma once

#include <chrono>

#include <platform/windows/event_processor.h>

namespace os_windows
{
    namespace event_timer
    {
        struct timer_controller;

        using timer_callback_t = std::function<void(timer_controller&)>;

        using timer_duration_t = std::chrono::milliseconds;

#pragma warning(push)
#pragma warning(disable : 26436) // non-virtual destructor 
        struct timer_controller
        {
            virtual void restart(timer_duration_t interval) noexcept = 0;

            virtual timer_duration_t interval() const noexcept = 0;

            virtual window_view window() const noexcept = 0;

            virtual void replace_callback(timer_callback_t callback) noexcept = 0;

            virtual void close() noexcept = 0;
        };
#pragma warning(pop)

        safe_event_processor create_timer(window_view window, timer_duration_t interval, timer_callback_t callback) noexcept;
    }

    using event_timer_controller = event_timer::timer_controller;
}