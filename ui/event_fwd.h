#pragma once

#include <ui/fwd.h>

namespace ui
{
    enum class event_style : D_CONDITIONAL_OS_WINDOWS(uint_t, int32_t)
    {
        null = D_CONDITIONAL_OS_WINDOWS(0x0000, -1),
        
#ifdef D_OS_WINDOWS
        size = 0x0005,
        mouse_double_click = 0x0203,
        mouse_wheel = 0x020A,
#endif

        mouse_move = D_CONDITIONAL_OS_WINDOWS(0x0200, 0x02),
        mouse_down = D_CONDITIONAL_OS_WINDOWS(0x0201, 0x00),
        mouse_up = D_CONDITIONAL_OS_WINDOWS(0x0202, 0x01),
    };

    template<event_style>
    class specialized_event;

#ifdef D_OS_WINDOWS
    using size_event = specialized_event<event_style::size>;
    using mouse_wheel = specialized_event<event_style::mouse_wheel>;
    using mouse_double_click = specialized_event<event_style::mouse_double_click>;
#endif

    using mouse_move_event = specialized_event<event_style::mouse_move>;
    using mouse_down_event = specialized_event<event_style::mouse_down>;
    using mouse_up_event = specialized_event<event_style::mouse_up>;
}