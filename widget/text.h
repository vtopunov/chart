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

        struct drawing_cache
        {
            static constexpr auto invalid_y = font::invalid_cursor.y();

        public:
            void clear() noexcept
            {
                texture_ = gl::sizes(std::move(texture_), 0_px, 0_px);
                y_ = invalid_y;
            }

            bool draw(buffer_view buffer, font::face_descriptor_t face, std::u8string_view text, pxsize2d sizes) noexcept;

            constexpr gl::texture2d_resources texture() const noexcept
            {
                return texture_;
            }

            constexpr font::fixed_point2d center() const noexcept
            {
                return 
                { 
                    font::fixed_t::instance(gl::width(texture_)) / 2, 
                    y_ 
                };
            }

            constexpr explicit operator bool() const noexcept
            {
                return invalid_y != y_;
            }

        private:
            gl::texture2d texture_{};
            font::fixed_t y_{ invalid_y };
        };

        template<class Widget>
        bool draw_to_cache(Widget& w, buffer_view buffer, pxsize2d sizes) noexcept
        {
            const auto has_text = !w.text.empty();

            if (has_text && !w.text_cache)
            {
                if (!w.font)
                {
                    w.font = font_cache::load_font(default_font_name, default_font_size);
                    if (D_UNLIKELY(!w.font)) D_ATTRIB_UNLIKELY
                    {
                        error_load_default_font_report();
                        return false;
                    }
                }

                return w.text_cache.draw(buffer, w.font, w.text, sizes);
            }

            return has_text;
        }

        template<class Widget>
        bool draw_to_cache(Widget& w, buffer_view buffer) noexcept
        {
            constexpr pxsize2d max_sizes{ fill_vec2(numeric_max_v<pxside_t>) };
            return draw_to_cache(w, buffer, max_sizes);
        }
    }
}
