#include "label.h"


namespace widget
{
    void label::operator()(redraw_event_type e) noexcept
    {
        if (text_cache.draw(e.get<temp_byte_buffer>(), font, text)) [[likely]]
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