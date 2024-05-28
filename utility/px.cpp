#include "px.h"

#include <core/buffer.h>

#include <debug/debug.h>

namespace px
{
    lumpixspan create_lumpixspan(buffer_view buffer, luminance_pixspace space) noexcept
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
            buffer.as_span<luminance_t>().data(),
            space
        };
    }
}