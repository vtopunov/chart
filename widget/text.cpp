#include "text.h"

#include <core/buffer.h>

#include <debug/debug.h>


namespace widget
{
    namespace text
    {
        namespace
        {
            pix8span create_pix8space(buffer_t& buffer, pix8space space) noexcept
            {
                const auto require_size_bytes = space.size_bytes();
                if (!buffer.try_reserve(require_size_bytes))
                {
                    const auto reserved_size_bytes = buffer.size_bytes();
                    w_debug("out of memory temp buffer: require {} bytes, reserved {} bytes", require_size_bytes, reserved_size_bytes);

                    const auto height = narrow_cast<pxside_t>((reserved_size_bytes / space.px_size) / space.line_size());
                    static_assert(!pix8space::dynamic_alignment_is_enabled);
                    space = { space.width(), height };
                }

                pix8span pixs{ buffer.as_ptr<pix8span::pixel_type>(), space };
                zero_memory(pixs);
                return pixs;
            }

            pix8span create_pix8space(buffer_t& buffer, pxside_t w, pxside_t h) noexcept
            {
                return create_pix8space(buffer, pix8space{ w, h });
            }
        }

        gl::texture2d draw_to_texture(buffer_t& buffer, font::face_descriptor_t face, std::u8string_view text, pxsize2d sizes) noexcept
        {
            const auto tm = font::text_metrics(face, text);

            const auto pixs = create_pix8space
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