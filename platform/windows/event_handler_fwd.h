#pragma once

#include <platform/windows/config.h>

namespace os_windows
{
    class event;

    LRESULT default_event_handler(const event& e) noexcept;

    using procedure_id_t = size_t;
}