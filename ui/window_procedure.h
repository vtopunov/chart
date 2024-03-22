#pragma once

#include <ui/fwd.h>


namespace ui
{
#ifdef D_OS_WINDOWS
    event_result_t D_OS_APICALL window_procedure
    (
        window_handle_t window,
        uint_t message,
        word_parameter_t word_parameter,
        long_parameter_t long_parameter
    ) noexcept;

#else
    constexpr auto window_procedure = def_window_proc;

#endif

}
