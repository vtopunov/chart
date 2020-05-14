#pragma once

#include <optional>

#include <core/unique_function.h>
#include <core/handle.h>

#include <platform/windows/defs.h>

namespace os_windows
{
    struct event_processor_view
    {
        window_view window;
        size_t id;
    };

    inline constexpr event_processor_view null_event_processor{ null_window, 0u };

    using event_callback_t = unique_function<std::optional<event_result_t>(const event&)>;
    using event_callback_factory_t = unique_function<event_callback_t(event_processor_view )>;

    bool exist(event_processor_view processor) noexcept;

    bool close(event_processor_view processor) noexcept;

    using safe_event_processor = shared_handle<event_processor_view>;

    safe_event_processor attach_event_processor(window_view window, event_callback_t callback) noexcept;
}