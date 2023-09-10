#include "label.h"


namespace widget
{
    void label::operator()(redraw_event_type e) noexcept
    {
        if (text::draw_to_cache(*this, e.get<buffer_view>())) [[likely]]
        {
            e.get<shaders::gray_texture_mix_color>().draw(position, text_cache.texture(), gl::colors::black_f);
        }
    }
}