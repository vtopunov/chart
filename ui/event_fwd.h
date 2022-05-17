#pragma once

#include <optional>

namespace ui
{
    class event;

    using event_result_t = ptrdiff_t;

    using event_callback_t = std::optional<event_result_t>(*)(void*, const event&);
}
