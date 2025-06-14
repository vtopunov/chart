#pragma once

#include <core/functional.h>
#include <core/null.h>

#include <ui/fwd.h>


namespace ui
{
    using event_callback_t = unique_function<event_result_opt_t(const event&)>;

    enum class event_processor_resource : size_t
    {};
}
