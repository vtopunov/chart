#include "event_timer.h"

#include <platform/windows/event.h>

namespace os_windows
{
    extern procedure_id_t generate_procedure_id() noexcept;

    extern event_dispatcher unsafe_register_event_handler(window_view window, procedure_id_t procedure_id, event_handler_t event_handler) noexcept;

    safe_timer create_timer(window_view window, ULONG_PTR timer_id, std::chrono::milliseconds timeount) noexcept
    {
        return timer
        {
            window,
            SetTimer(
                window.handle_,
                narrow_cast<ULONG_PTR>(timer_id),
                narrow_cast<ULONG>(timeount.count()),
                nullptr
            )
        };
    }

    LRESULT timer_event_handler::operator()(const event& processed_event) noexcept
    {
        if (const auto timer_event = processed_event.as<event_type::timer>())
        {
            if (timer_event->timer_id() == timer->timer_id())
            {
                assert(timer_event->window_handle() == timer->window_handle());

                callback();
            }
        }

        return 0;
    }

    safe_event_dispatcher register_event_timer(window_view window, std::chrono::milliseconds timeount, event_timer_callback_t callback) noexcept
    {
        const auto procedure_id = generate_procedure_id();

        event_dispatcher dispatcher;

        if (const auto timer_handle = create_timer(window, narrow_cast<ULONG_PTR>(procedure_id), timeount))
        {
            dispatcher = unsafe_register_event_handler(window, procedure_id,
                timer_event_handler
                {
                    timer_handle,
                    std::move(callback)
                }
            );
        }

        return dispatcher;
    }
}
