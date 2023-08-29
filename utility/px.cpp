#include "px.h"

#include <core/buffer.h>

#include <debug/debug.h>

namespace px
{
    pix8span zeros_pix8space(buffer_view buffer, pix8space space) noexcept
    {
        {
            const auto space_size_bytes = space.size_bytes();
            const auto buffer_size_bytes = size_bytes(buffer);

            if (space_size_bytes > buffer_size_bytes) [[unlikely]]
            {
                w_debug("out of buffer: require {} bytes, reserved {} bytes", space_size_bytes, buffer_size_bytes);

                const auto height = narrow<pxside_t>((buffer_size_bytes / space.px_size) / space.line_size());
                static_assert(!pix8space::dynamic_alignment_is_enabled);
                space = { space.width(), height };
            }
        }

        pix8span pixs{ buffer.as_ptr<pix8span::pixel_type>(), space };
        zero_memory(pixs);
        return pixs;
    }
}