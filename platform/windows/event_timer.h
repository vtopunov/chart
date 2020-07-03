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

        struct timer_view
        {
            window_handle_t window_handle;
            size_t id;
            timer_duration_t interval;
        };

        constexpr auto invalid_timer_duration = timer_duration_t::zero();

        constexpr bool valid(timer_view timer) noexcept
        {
            return timer.id && timer.interval > invalid_timer_duration;
        }

        bool close(timer_view timer) noexcept;

        using safe_timer = unique_handle<timer_view>;

        safe_timer create_timer(window_handle_t window_handle, size_t id, timer_duration_t interval) noexcept;

        safe_timer create_timer(window_view window, timer_duration_t interval) noexcept;

        safe_timer set_timer_interval(safe_timer timer, timer_duration_t interval) noexcept;

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
    using safe_event_timer = event_timer::safe_timer;
}