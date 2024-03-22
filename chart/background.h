#pragma once

#include <chart/shader.h>


namespace chart
{
    struct background
    {
        rgbaf_color_t color{ colors::white_f };

        void operator()(const shader::background_user& shdr) const noexcept
        {
            shdr.color(color)
                .draw();
        }
    };
}
