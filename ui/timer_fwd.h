#pragma once

#include <core/null.h>

namespace ui
{    
    enum class timer_resource : size_t
    {};

    using nulltimer_t = null_t<timer_resource>;
    constexpr nulltimer_t nulltimer{};
}