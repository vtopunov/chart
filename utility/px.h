#pragma once

#include <core/buffer_view.h>

#include <px/pixspan.h>


namespace px
{
    pix8span create_pix8span(buffer_view buffer, pix8space space) noexcept;
   
    inline pix8span create_zeros_pix8span(buffer_view buffer, pix8space space) noexcept
    {
        const auto pixs = create_pix8span(buffer, space);
        zero_memory(pixs);
        return pixs;
    }

    inline pix8span create_zeros_pix8span(buffer_view buffer, pxside_t w, pxside_t h) noexcept
    {
        return create_zeros_pix8span(buffer, pix8space{ w, h });
    }
}
