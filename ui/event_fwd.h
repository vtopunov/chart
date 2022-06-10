#pragma once

#include <ui/ufwd.h>

namespace ui
{
    enum class event_style : D_CONDITIONAL_OS_WINDOWS(uint_t, int32_t)
    {
        null = D_CONDITIONAL_OS_WINDOWS(0x0000, -1),
        
#ifdef D_OS_WINDOWS
        size = 0x0005,
        close = 0x0010,
        mouse_move = 0x0200,
        mouse_lbutton_down = 0x0201,
        mouse_lbutton_up = 0x0202,
        mouse_lbutton_double_click = 0x0203
#endif
    };

    template<event_style>
    class specialized_event;

#ifdef D_OS_WINDOWS
    using size_event = specialized_event<event_style::size>;
    using close_event = specialized_event<event_style::close>;
    using mouse_move_event = specialized_event<event_style::mouse_move>;
    using mouse_lbutton_down_event = specialized_event<event_style::mouse_lbutton_down>;
    using mouse_lbutton_up_event = specialized_event<event_style::mouse_lbutton_up>;
#endif
}