#pragma once

#include <shader/library.h>


namespace chart
{
    struct background
    {
        using brush_type = rgbaf_color;
        static constexpr brush_type default_brush{ colors::white_f };

        brush_type brush{ default_brush };

        void operator()(const shader_embed::colored_rectangle& shdr) const noexcept
        {
            shdr.color(brush);
            shdr.draw();
        }
    };
}
