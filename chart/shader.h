#pragma once

#include <widget/shader.h>


namespace chart
{
    namespace shader
    {
        using background_shader = widget::shader::colored_rectangle;
        using pix8_figure_shader = widget::shader::luminance8_texture_mix_color;

        struct background_shader_user : widget_shader_user_for_t<background_shader_user, background_shader>
        {};

        struct pix8_figure_shader_user : widget_shader_user_for_t<pix8_figure_shader_user, pix8_figure_shader>
        {};
    }
}