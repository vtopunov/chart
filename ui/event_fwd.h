#pragma once

#include <optional>

#include <core/os.h>

namespace ui
{
    class event;

    using event_result_t = LRESULT;

    using event_callback_t = std::optional<event_result_t>(*)(void*, const event&);
}
