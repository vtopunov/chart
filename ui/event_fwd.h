#pragma once

#include <optional>

#include <os/os.h>

namespace ui
{
    class event;

    using event_result_t = LRESULT;

    using event_callback_t = std::optional<event_result_t>(*)(void*, const event&);
}
