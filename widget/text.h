#pragma once

#include <core/buffer_fwd.h>

#include <gl/texture.h>

#include <utility/font_cache.h>


namespace widget
{
    namespace text
    {
        constexpr auto default_font_name = _PATH("OpenSans-Regular.ttf");
        constexpr auto default_font_size = 15_px;


        gl::texture2d draw_to_texture(buffer_t& buffer, font::face_descriptor_t face, std::u8string_view text, pxsize2d sizes) noexcept;

        inline gl::texture2d draw_to_texture(buffer_t& buffer, font::face_descriptor_t face, std::u8string_view text) noexcept
        {
            constexpr pxsize2d max_sizes{ fill_vec2(numeric_max_v<pxside_t>) };
            return draw_to_texture(buffer, face, text, max_sizes);
        }
    }
}
