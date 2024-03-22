#include "label.h"


namespace widget
{
    void label::operator()(redraw_event_type e) noexcept
    {
        if (text::draw_to_cache(*this, e.get<buffer_view>())) [[likely]]
        {
            e.get<shader::luminance8_texture_mix_color>()
                .use()
                .position(position)
                .color(colors::black_f)
                .texture(text_cache.texture())
                .draw();
        }
    }
}