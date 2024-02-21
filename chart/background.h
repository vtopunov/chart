#pragma once

#include <chart/shader.h>


namespace chart
{
    struct background
    {
        rgba_colorf_t color{ colors::white_f };

        void operator()(const shader::background_shader_user& shdr) const noexcept
        {
            shdr.store(color)
                .draw();
        }
    };
}
