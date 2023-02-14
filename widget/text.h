#pragma once

#include <core/buffer_view.h>

#include <gl/texture.h>

#include <utility/font_cache.h>

namespace widget
{
    namespace text
    {
        constexpr auto default_font_name = _PATH("OpenSans-Regular.ttf");
        constexpr auto default_font_size = 15_px;

        void error_load_default_font_report() noexcept;
      
        gl::texture2d draw_to_texture(buffer_view buffer, font::face_descriptor_t face, std::u8string_view text, pxsize2d sizes) noexcept;

        template<class Widget>
        void draw_to_cache(Widget& w, buffer_view buffer, pxsize2d sizes) noexcept
        {
            if (!w.texture_text_cache && !w.text.empty())
            {
                if (!w.font)
                {
                    w.font = font_cache::load_font(text::default_font_name, text::default_font_size);
                    if (D_UNLIKELY(!w.font)) D_ATTRIB_UNLIKELY
                    {
                        error_load_default_font_report();
                        return;
                    }
                }

                w.texture_text_cache = text::draw_to_texture(buffer, w.font, w.text, sizes);
            }
        }

        template<class Widget>
        void draw_to_cache(Widget& w, buffer_view buffer) noexcept
        {
            constexpr pxsize2d max_sizes{ fill_vec2(numeric_max_v<pxside_t>) };
            draw_to_cache(w, buffer, max_sizes);
        }
    }
}
