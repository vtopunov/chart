#pragma once

#include <gl/texture.h>

#include <utility/font_cache.h>

namespace widget
{
    namespace text
    {
        constexpr auto default_font_name = _PATH("OpenSans-Regular.ttf");
        constexpr auto default_font_size = 15_px;


        gl::texture2d draw_to_texture(buffer_t& temp_buffer, font::face_descriptor_t face, std::u8string_view text, px::size2d sizes) noexcept;

        inline gl::texture2d draw_to_texture(buffer_t& temp_buffer, font::face_descriptor_t face, std::u8string_view text) noexcept
        {
            constexpr px::size2d max_sizes{ fill_vec2(numeric_max_v<pxside_t>) };
            return draw_to_texture(temp_buffer, face, text, max_sizes);
        }
    }
}
