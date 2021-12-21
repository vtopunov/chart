#pragma once

#include <core/null.h>
#include <core/os.h>

namespace ui
{    
    enum class timer_resource : UINT_PTR
    {};

    using nulltimer_t = null_t<timer_resource>;
    inline constexpr nulltimer_t nulltimer{};
}