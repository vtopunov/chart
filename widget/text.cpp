#include "text.h"

#include <core/buffer.h>

#include <debug/debug.h>

#include <utility/px.h>

namespace widget
{
    namespace text
    {
        void error_load_default_font_report() noexcept
        {
            e_debug("error load default font");
        }

        gl::texture2d draw_to_texture(buffer_view buffer, font::face_descriptor_t face, std::u8string_view text, pxsize2d sizes) noexcept
        {
            const auto tm = font::text_metrics(face, text);

            const auto pixs = px::zeros_pix8space
            (
                buffer,
                std::min(sizes.width(), ceil_to<pxside_t>(tm.width)),
                std::min(sizes.height(), ceil_to<pxside_t>(tm.bottom - tm.top))
            );

            font::draw_text(pixs, 0_px, -tm.top, face, text);

            auto texture = gl::create_texture2d(pixs);
            if (!texture)
            {
                e_debug("create text texture error: {}", glGetError());
                return {};
            }

            return texture;
        }
    }
}