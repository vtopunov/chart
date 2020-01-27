#pragma once

#include <chrono>

#include <platform/windows/event_handler.h>

namespace os_windows
{
    struct timer_controller;

    using event_timer_callback_t = std::function<void(timer_controller&)>;

#pragma warning(push)
#pragma warning(disable : 26436) // non-virtual destructor 
    struct timer_controller
    {
        virtual bool restart(std::chrono::milliseconds timeout) noexcept = 0;

        virtual std::chrono::milliseconds interval() const noexcept = 0;

        virtual window_view window() const noexcept = 0;

        virtual void replace_callback(event_timer_callback_t callback) noexcept = 0;

        virtual bool close() noexcept = 0;
    };
#pragma warning(pop)

    safe_event_handler register_timer(window_view window, std::chrono::milliseconds interval, event_timer_callback_t callback) noexcept;
}