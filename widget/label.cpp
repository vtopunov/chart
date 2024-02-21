#include "label.h"


namespace widget
{
    void label::operator()(redraw_event_type e) noexcept
    {
        if (text::draw_to_cache(*this, e.get<buffer_view>())) [[likely]]
        {
            e.get<shader::luminance8_texture_mix_color>()
                .use()
                .store(position)
                .store(colors::black_f)
                .store(text_cache.texture())
                .draw();
        }
    }
}