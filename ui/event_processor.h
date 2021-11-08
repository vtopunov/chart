#pragma once


#include <core/resouce.h>
#include <ui/window_fwd.h>
#include <ui/event_fwd.h>

namespace ui
{
    enum class event_processor_resource : size_t
    {
        null
    };

    using nulleventprocessor_t = null_t<event_processor_resource>;

    inline constexpr nulleventprocessor_t nulleventprocessor{};

    static_assert(nulleventprocessor == event_processor_resource::null);

    bool close(event_processor_resource processor) noexcept;

    using event_processor_t = unique_resource<event_processor_resource>;

    [[nodiscard]]
    event_processor_t create_event_processor(window_resource window, void* data, event_callback_t callback) noexcept;
}