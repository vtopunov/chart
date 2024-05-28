#pragma once

#include <core/buffer_view.h>

#include <px/pixspan.h>


namespace px
{
    lumpixspan create_lumpixspan(buffer_view buffer, luminance_pixspace space) noexcept;
   
    inline lumpixspan create_zeros_lumpixspan(buffer_view buffer, luminance_pixspace space) noexcept
    {
        const auto pixs = create_lumpixspan(buffer, space);
        zero_memory(pixs);
        return pixs;
    }

    inline lumpixspan create_zeros_lumpixspan(buffer_view buffer, npx_t w, npx_t h) noexcept
    {
        return create_zeros_lumpixspan(buffer, luminance_pixspace{ w, h });
    }
}
