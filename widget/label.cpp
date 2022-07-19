#include "label.h"

#include <debug/debug.h>

#include <egl_ui/egl_resources.h>

#include <widget/shader.h>
#include <widget/text.h>


namespace widget
{
    bool label::initialize(const egl_resources& egl) noexcept
    {
        if (!shader::gray_texture_mix_color::initialize(sizes(egl)))
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

    void label::draw(buffer_t& buffer) noexcept
    {
        if (!texture_text_cache && !text.empty())
        {
            texture_text_cache = text::draw_to_texture(buffer, font, text);
        }

        if (texture_text_cache)
        {
            shader::gray_texture_mix_color::draw(position, texture_text_cache, gl::colors::black_f);
        }
    }
}