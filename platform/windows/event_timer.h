#pragma once

#include <chrono>

#include <platform/windows/event_handler.h>

namespace os_windows
{
    using event_timer_callback_t = std::function<void()>;

    class timer
    {
    public:
        constexpr timer() noexcept = default;

        constexpr timer(window_view window, ULONG_PTR timer_id) noexcept
            : window_{ window }
            , timer_id_{ timer_id }
        {}

        constexpr bool is_valid() const noexcept
        {
            return timer_id_ != 0u;
        }

        constexpr LONG_PTR timer_id() const noexcept
        {
            return timer_id_;
        }

        constexpr HWND window_handle() const noexcept
        {
            return window_.handle_;
        }

        void close() noexcept
        {
            if (const auto timer_id = std::exchange(timer_id_, 0u); timer_id)
            {
                const auto is_success = KillTimer(window_.handle_, timer_id);
                is_success; assert(is_success != FALSE);
            }
        }

    private:
        window_view window_{ nullptr };
        ULONG_PTR timer_id_{ 0u };
    };

    using safe_timer = safe_handle<timer>;

    safe_timer create_timer(window_view window, ULONG_PTR timer_id, std::chrono::milliseconds timeount) noexcept;

    struct timer_event_handler
    {
        safe_timer timer;
        event_timer_callback_t callback;

        LRESULT operator () (const event& processed_event) noexcept;
    };

    safe_event_dispatcher register_event_timer(window_view window, std::chrono::milliseconds timeount, event_timer_callback_t callback) noexcept;
}