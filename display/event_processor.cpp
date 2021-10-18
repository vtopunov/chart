#include "event_processor.h"

#include <display/event_processors_container.h>

namespace display
{
    namespace event_processor
    {
        bool close(processor_resource processor) noexcept
        {
            return processors_container_global().erase(processor);
        }

        processor_t create_processor(window_resource window, void* data, event_callback_t callback) noexcept
        {
            return
            {
                resource_construct,
                processors_container_global().insert(window, data, callback)
            };
        }
    }
}