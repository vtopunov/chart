#pragma once

#include <core/resource.h>

#include <ui/event_processor_fwd.h>


namespace ui
{
    bool destroy_processor(event_processor_resource processor) noexcept;

    struct event_processor_resource_deleter
    {
        void operator()(event_processor_resource processor) const noexcept
        {
            destroy_processor(processor);
        }
    };

    using event_processor = unique_resource<event_processor_resource, event_processor_resource_deleter>;

    [[nodiscard]]
    event_processor create_event_processor(window_handle_t window, event_callback_t callback) noexcept;
}