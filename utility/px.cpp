#include "px.h"

#include <core/buffer.h>

#include <debug/debug.h>

namespace px
{
    pix8span create_pix8span(buffer_view buffer, pix8space space) noexcept
    {
        {
            const auto space_size_bytes = space.size_bytes();
            const auto buffer_size_bytes = size_bytes(buffer);

            if (space_size_bytes > buffer_size_bytes) [[unlikely]]
            {
                e_debug("out of buffer: require {} bytes, reserved {} bytes", space_size_bytes, buffer_size_bytes);
                return {};
            }
        }

        return 
        {
            buffer.as_span<pix8span::pixel_type>().data(),
            space
        };
    }
}