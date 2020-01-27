#pragma once

#include <core/shared_handle.h>

#include <platform/windows/defs.h>

namespace os_windows
{
    class event_handler
    {
    public:
        constexpr event_handler() noexcept = default;

        constexpr event_handler(window_view window, size_t id) noexcept
            : view_{ window, id }
        {}

        bool close() noexcept;

        bool is_valid() const noexcept;

        constexpr event_handler_view view() const noexcept
        {
            return view_;
        }

    private:
        event_handler_view view_{ null_event_handler_view };
    };

    using safe_event_handler = shared_handle<event_handler>;

    safe_event_handler register_event_handler_factory(window_view window, event_callback_factory callback_factory) noexcept;

    safe_event_handler register_event_handler(window_view window, event_callback_function callback) noexcept;

    bool unregister_event_handler(event_handler_view view) noexcept;
}