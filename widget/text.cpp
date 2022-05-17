#include "text.h"

#include <debug/debug.h>

namespace widget
{
    namespace text
    {
        gl::texture2d draw_to_texture(buffer_t& temp_buffer, font::face_descriptor_t face, std::u8string_view text, px::size2d sizes) noexcept
        {
            const auto tm = font::text_metrics(face, text);

            const pix8space space
            {
                std::min(sizes.width(), tm.width.narrow_to_ceil<pxside_t>()),
                std::min(sizes.height(), (tm.bottom - tm.top).narrow_to_ceil<pxside_t>())
            };

            const auto size_bytes = space.size_bytes();
            if (!temp_buffer.try_resize(size_bytes))
            {
                e_debug("out of memory\n");
                return {};
            }

            pix8span pixs{ temp_buffer.as_ptr<pix8span::pixel_type>(), space };
            zero_memory(pixs);

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