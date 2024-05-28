#pragma once

#include <core/functional.h>
#include <core/null.h>

#include <ui/fwd.h>


namespace ui
{
    using event_callback_t = unique_function<event_result_opt_t(const event&)>;

    enum class event_processor_resource : size_t
    {
        null
    };

    using nulleventprocessor_t = null_t<event_processor_resource>;
    constexpr nulleventprocessor_t nulleventprocessor{};

    static_assert(nulleventprocessor == event_processor_resource::null);
}
