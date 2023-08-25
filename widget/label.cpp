#include "label.h"

#include <widget/window_configation.h>
#include <widget/event.h>
#include <widget/text.h>


namespace widget
{
    window_configation label::operator()(const init_event&) const noexcept
    {
        return enable_gray_texture_mix_color_shdr
             | enable_pix8_temp_buffer;
    }

    void label::operator()(const redraw_event& e) noexcept
    {
        if (text::draw_to_cache(*this, e)) [[likely]]
        {
            e.shaders.gray_texture_mix_color.draw(position, text_cache.texture(), gl::colors::black_f);
        }
    }
}