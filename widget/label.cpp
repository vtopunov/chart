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
            const auto& s = e.get<shader_embed::luminance_texture>();
            
            s.use();
            s.position(position);
            s.color(colors::black_f);
            s.texture(text_cache.texture());
            s.draw();
        }
    }
}