#pragma once

#include <widget/shader.h>


namespace chart
{
    namespace shader
    {
        using background = widget::shader::colored_rectangle;
        using pix8_figure = widget::shader::luminance8_texture_mix_color;

        struct background_user : widget_shader_user_for_t<background_user, background>
        {};

        struct pix8_figure_user : widget_shader_user_for_t<pix8_figure_user, pix8_figure>
        {};
    }
}