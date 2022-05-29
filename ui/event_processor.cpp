#include "event_processor.h"

#include <ui/event_processors_container.h>

namespace ui
{
    bool close(event_processor_resource processor) noexcept
    {
        return event_processors_global().erase(processor);
    }

    event_processor create_event_processor(window_handle_t window, void* data, event_callback_t callback) noexcept
    {
        return
        {
            resource_construct,
            event_processors_global().insert(window, data, callback)
        };
    }
}