#include "temp_buffer.h"

#include <px/pixspace.h>

#include <debug/debug.h>


namespace widget
{
    bool temp_buffer::operator()(viewport_event<> e) noexcept
    {
        const auto require_size_bytes
            = luminance_pixspace{ e.viewport() }.size_bytes();

        if (!try_reserve(require_size_bytes)) [[unlikely]]
        {
            e_debug("out of memory temp buffer: require {} bytes", require_size_bytes);
            return false;
        }

        return true;
    }
}