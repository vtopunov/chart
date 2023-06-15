#pragma once

#include <core/null.h>

#include <ui/fwd.h>


namespace ui
{
    enum class event_processor_resource : size_t
    {
        null
    };

    using nulleventprocessor_t = null_t<event_processor_resource>;
    constexpr nulleventprocessor_t nulleventprocessor{};

    static_assert(nulleventprocessor == event_processor_resource::null);
}
