#include "temp_buffer.h"

#include <px/pixspace.h>

#include <ui/debug.h>


namespace widget
{
    basic_buffer_view<luminance_t> temp_byte_buffer::try_get_reserve(size_t size_bytes) noexcept
    {
        if (buffer_.try_reserve(size_bytes)) [[likely]]
        {
            return buffer_;
        }
        else
        {
            ui_fatal_debug("out of memory\n");;
            return {};
        }
    }

    lumpixspan temp_byte_buffer::image(luminance_pixspace space) noexcept
    {
        if (const auto view = try_get_reserve(space.size_bytes())) [[likely]]
        {
            return lumpixspan{ view.data(), space};
        }
        else
        {
            return {};
        }
    }

#ifdef D_OS_WINDOWS
    void temp_byte_buffer::operator () (const ui::size_event& e) noexcept
    {
        const auto size_bytes = luminance_pixspace{ e.sizes() }.size_bytes();
        const auto buffer_size_limit = buffer_.size() / 2u;
        if(size_bytes < buffer_size_limit)  [[unlikely]]
        {
            constexpr size_t min_limit{ 1024 };
            if (buffer_size_limit > min_limit)
            {
                if (byte_buffer::good_size(size_bytes) < buffer_size_limit)
                {
                    buffer_.reset();
                }
            }
        }
    }

#endif
}