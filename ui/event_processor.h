#pragma once

#include <core/resouce.h>

#include <ui/fwd.h>

namespace ui
{
    enum class event_processor_resource : size_t
    {
        null
    };

    using nulleventprocessor_t = null_t<event_processor_resource>;
    constexpr nulleventprocessor_t nulleventprocessor{};

    static_assert(nulleventprocessor == event_processor_resource::null);

    bool close(event_processor_resource processor) noexcept;

    struct event_processor_resource_deleter
    {
        void operator()(event_processor_resource processor) const noexcept
        {
            close(processor);
        }
    };

    using event_processor = unique_resource<event_processor_resource, event_processor_resource_deleter>;

    [[nodiscard]]
    event_processor create_event_processor(window_handle_t window, void* data, event_callback_t callback) noexcept;
}