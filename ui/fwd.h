#pragma once

#include <optional>

#include <core/unique_function.h>

#include <os/fwd.h>


namespace ui
{
    using os::window_handle_t;
    using os::module_handle_t;
    using os::const_module_handle_t;

#ifdef D_OS_WINDOWS
    using os::uint_t;
    using os::dword_t;
    using os::word_t;
#endif

    class event;

    using event_result_t = ptrdiff_t;
    using event_result_opt_t = D_CONDITIONAL_OS_WINDOWS(std::optional<event_result_t>, std::nullopt_t);
    using event_callback_t = unique_function<event_result_opt_t (const event&)>;
}
