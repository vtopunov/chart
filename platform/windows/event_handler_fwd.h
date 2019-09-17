#pragma once

#include <platform/windows/config.h>

namespace os_windows
{
    class event;

    LRESULT default_event_handler(const event& e) noexcept;

    using procedure_id_t = size_t;

    constexpr procedure_id_t invaid_procedure_id = std::numeric_limits<procedure_id_t>::max();

    constexpr bool is_valid_procedure_id(procedure_id_t procedure_id) noexcept
    {
        return procedure_id != invaid_procedure_id;
    }
}