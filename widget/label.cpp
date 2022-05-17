#include "label.h"

#include <debug/debug.h>

#include <widget/shader.h>
#include <widget/text.h>


namespace widget
{
    bool label::initialize(px::size2d viewport_sizes) noexcept
    {
        if (!shader::gray_texture_mix_color::initialize(viewport_sizes))
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

    void label::draw(buffer_t& temp_buffer) noexcept
    {
        if (!text_texture && !text.empty())
        {
            text_texture = text::draw_to_texture(temp_buffer, font, text);
        }

        if (text_texture)
        {
            shader::gray_texture_mix_color::draw(position, text_texture, gl::colors::black_f);
        }
    }
}