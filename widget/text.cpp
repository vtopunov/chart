#include "text.h"

#include <core/buffer.h>

#include <debug/debug.h>

#include <utility/px.h>

using namespace std::string_view_literals;


namespace widget
{
    namespace text
    {
        namespace
        {
            [[nodiscard]] font::metrics initial_tm(font::face_descriptor_t face) noexcept
            {
                auto tm = font::text_metrics(face, u8"Ap"sv);
                tm.width = {};
                return tm;
            };
        }

        void error_load_default_font_report() noexcept
        {
            e_debug("error load default font");
        }

        bool drawing_cache::draw(buffer_view buffer, font::face_descriptor_t face, std::u8string_view text, pxsize2d sizes) noexcept
        {
            const auto tm = font::text_metrics(initial_tm(face), face, text);
            if (D_UNLIKELY(!tm)) D_ATTRIB_UNLIKELY
            {
                return false;
            }

            const auto pixs = px::zeros_pix8space
            (
                buffer,
                std::min(sizes.width(), ceil_to<pxside_t>(tm.width)),
                std::min(sizes.height(), ceil_to<pxside_t>(tm.bottom - tm.top))
            );

            const auto y_cursor = font::draw_text(pixs, 0_px, -tm.top, face, text).y();
            if (D_UNLIKELY(invalid_y == y_cursor)) D_ATTRIB_UNLIKELY
            {
                return false;
            }

            if (texture_)
            {
                texture_ = gl::write(std::move(texture_), pixs);
            }
            else
            {
                texture_ = gl::create_texture2d(pixs);
            }

            if (D_UNLIKELY(gl::sizes(texture_) != pixs.sizes())) D_ATTRIB_UNLIKELY
            {
                return false;
            }
            
            const auto height = font::fixed_t::instance(pixs.height());
            y_ = std::clamp(y_cursor, {}, height) / 2;
            return true;
        }
    }
}