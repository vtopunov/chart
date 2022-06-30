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
                std::min(sizes.width(), ceil_to<pxside_t>(tm.width)),
                std::min(sizes.height(), ceil_to<pxside_t>(tm.bottom - tm.top))
            };

            const auto size_bytes = space.size_bytes();
            if (!temp_buffer.try_resize(size_bytes))
            {
                e_debug("out of memory");
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