#pragma once

#include <widget/shader.h>


namespace chart
{
    namespace shader
    {
        using background = widget::shader::colored_rectangle;
        using luminance_figure = widget::shader::luminance_texture_mix_color;

        using background_user = background::shader_user_type;
        using luminance_figure_user = luminance_figure::shader_user_type;
    }
}