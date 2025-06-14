#include "temp_buffer.h"

#include <px/pixspace.h>

#include <ui/debug.h>


namespace widget
{
    lumpixspan temp_byte_buffer::image(luminance_pixspace space) noexcept
    {
        const auto size_bytes = space.size_bytes();
        if (!buffer_.try_reserve(size_bytes)) [[unlikely]]
        {
            ui_fatal_debug("out of memory\n");
            return {};
        }

        return lumpixspan{ static_cast<luminance_t*>(buffer_.void_data()), space };
    }

#ifdef D_OS_WINDOWS   
    void temp_byte_buffer::operator () (const ui::size_event& e) noexcept
    {
        const auto size_bytes = luminance_pixspace{ e.sizes() }.size_bytes();
        const auto buffer_size_limit = buffer_.size() / 2u;
        if(size_bytes < buffer_size_limit)  [[unlikely]]
        {
            if(byte_buffer::good_size(size_bytes) < buffer_size_limit)
            {
                buffer_.reset();
            }
        }
    }

#endif
}