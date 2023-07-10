#include "label.h"

#include <widget/window.h>
#include <widget/text.h>


namespace widget
{
    void label::operator()(widget_initializer& ini) const noexcept
    {
        ini.cfg()
            .gray_texture_mix_color_shdr()
            .pix8_temp_buffer();
    }

    void label::draw(const window& w) noexcept
    {
        if (D_LIKELY(text::draw_to_cache(*this, w.temp_buffer_view()))) D_ATTRIB_LIKELY
        {
            w.shaders.gray_texture_mix_color.draw(position, text_cache.texture(), gl::colors::black_f);
        }
    }
}