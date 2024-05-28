#include "label.h"


namespace widget
{
    bool label::operator()(widget::basic_initialization_event<>) noexcept
    {
        if(!font)
        {
            font = font_cache::default_font();
        }

        return !!font;
    }

    void label::operator()(redraw_event_type e) noexcept
    {
        if (text_cache.draw(e.get<buffer_view>(), font, text)) [[likely]]
        {
            e.get<shader::luminance_texture_mix_color>()
                .use()
                .position(position)
                .color(colors::black_f)
                .texture(text_cache.texture())
                .draw();
        }
    }
}