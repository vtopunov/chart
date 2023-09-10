#include "draw_context_elements.h"

#include <px/pixspace.h>

#include <debug/debug.h>


namespace widget
{
    bool pix8_temp_buffer::operator()(viewport_size2d viewport) noexcept
    {
        const auto require_size_bytes
            = pix8space{ viewport }.size_bytes();

        if (!try_reserve(require_size_bytes)) [[unlikely]]
        {
            e_debug("out of memory temp buffer: require {} bytes", require_size_bytes);
            return false;
        }

        return true;
    }
}