#pragma once

#include <widget/event.h>
#include <widget/shader.h>
#include <widget/user_gesture.h>


namespace chart
{
    using widget::content_size2d;
    using widget::event_result;

    namespace shader
    {
        using namespace widget::shader;
    }


    using mouse_wheel_event_t = widget::mouse_wheel_event<content_size2d>;

    using mouse_move_event_t = widget::mouse_move_event<
        ui::user_gesture,
        content_size2d
    >;

    using mouse_double_click_event_t = widget::mouse_double_click_event<
        content_size2d
    >;

    using redraw_event_t = widget::redraw_event<
        shader::gray_texture_mix_color,
        shader::colored_rectangle,
        buffer_view,
        content_size2d
    >;
}