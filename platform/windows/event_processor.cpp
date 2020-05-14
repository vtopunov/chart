#include "event_processor.h"

#include <platform/windows/event_processors_container.h>

namespace os_windows
{
    namespace
    {
        struct default_callback_factory
        {
            event_callback_t callback;

            event_callback_t operator () (event_processor_view) noexcept
            {
                return std::move(callback);
            }
        };
    }

    bool exist(event_processor_view processor) noexcept
    {
        const auto position = const_event_processors_container_global().find(processor);
        return position.first != position.last;
    }

    bool close(event_processor_view processor) noexcept
    {
        return event_processors_container_global().erase(processor);
    }

    safe_event_processor attach_event_processor(window_view window, event_callback_t callback) noexcept
    {
        return
        {
            handle_construct,
            window,
            event_processors_container_global().insert(window, default_callback_factory{ std::move(callback) })
        };
    }
}
