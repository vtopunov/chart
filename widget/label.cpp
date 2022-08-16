#include "label.h"

#include <debug/debug.h>

#include <widget/window.h>
#include <widget/text.h>


namespace widget
{
    bool label::initialize(window& w) noexcept
    {
        if (!w.shaders.gray_texture_mix_color.initialize(sizes(w)))
        {
            e_debug("shaders error: {}", glGetError());
            return false;
        }

        if (!font)
        {
            font = font_cache::load_font(text::default_font_name, text::default_font_size);
            if (!font)
            {
                e_debug("can't create font");
                return false;
            }
        }

        return true;
    }

    void label::draw(const window& w) noexcept
    {
        if (!texture_text_cache && !text.empty())
        {
            texture_text_cache = text::draw_to_texture(w.temp_buffer, font, text);
        }

        if (texture_text_cache)
        {
            w.shaders.gray_texture_mix_color.draw(position, texture_text_cache, gl::colors::black_f);
        }
    }
}