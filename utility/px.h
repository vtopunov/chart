#pragma once

#include <core/buffer_view.h>

#include <px/pixspan.h>

namespace px
{
    pix8span zeros_pix8space(buffer_view buffer, pix8space space) noexcept;

    inline pix8span zeros_pix8space(buffer_view buffer, pxside_t w, pxside_t h) noexcept
    {
        return zeros_pix8space(buffer, pix8space{ w, h });
    }
}