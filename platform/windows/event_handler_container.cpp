#include "event_handler_container.h"

namespace os_windows
{
    size_t generate_event_handler_id() noexcept
    {
        static size_t id = 0;
        return ++id;
    }

    event_handler_container& event_handler_container_global() noexcept
    {
        static event_handler_container handlers;
        return handlers;
    }
}
