#pragma once

#include <core/resource.h>
#include <core/unique_function.h>

#include <ui/fwd.h>


namespace ui
{
    using event_callback_t = unique_function<event_result_opt_t(const event&)>;

    enum class event_processor_resource : size_t
    {};

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