#include "event_processor.h"

#include <ui/event_processors_storage.h>

namespace ui
{
    void destroy_processor(event_processor_resource processor) noexcept
    {
        event_processors_global().destroy_processor(processor);
    }

    event_processor create_event_processor(window_handle_t window, event_callback_t callback) noexcept
    {
        return event_processor{ event_processors_global().create(window, std::move(callback)) };
    }
}