#include "temp_buffer.h"

#include <px/pixspace.h>

#include <debug/debug.h>


namespace widget
{
    bool pix8_temp_buffer::operator()(viewport_event<> e) noexcept
    {
        const auto require_size_bytes
            = pix8space{ e.viewport() }.size_bytes();

        if (!try_reserve(require_size_bytes)) [[unlikely]]
        {
            e_debug("out of memory temp buffer: require {} bytes", require_size_bytes);
            return false;
        }

        return true;
    }
}