#include "label.h"

#include <widget/window.h>
#include <widget/text.h>


namespace widget
{
    void label::operator()(window_configuration& cfg) const noexcept
    {
        cfg.build()
            .colored_rectangle_shdr()
            .pix8_temp_buffer();
    }

    void label::draw(const window& w) noexcept
    {
        text::draw_to_cache(*this, w.temp_buffer_view());

        if (texture_text_cache)
        {
            w.shaders.gray_texture_mix_color.draw(position, texture_text_cache, gl::colors::black_f);
        }
    }
}