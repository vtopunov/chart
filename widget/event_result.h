#pragma once

#include <cstddef>


namespace widget
{
    enum class event_result : size_t
    {
        idle = 0,
        redraw = (1 << 0)
    };
}