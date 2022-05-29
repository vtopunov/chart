#pragma once

#include <optional>

#include <os/osfwd.h>

namespace ui
{
    using window_handle_t = os::window_handle_t;
    static_assert(std::is_class_v<std::remove_pointer_t<window_handle_t>>);

    using module_handle_t = os::module_handle_t;

    class event;

    using event_result_t = ptrdiff_t;

    using event_callback_t = std::optional<event_result_t>(*)(void*, const event&);
}
