#pragma once

#include <px/pxfwd.h>

namespace widget
{
    struct rectangle
    {
        px::point2d position{};
        px::size2d sizes{};

        constexpr bool contains(const px::point2d& p) const noexcept
        {
            return contains(p.x(), position.x(), sizes.width())
                && contains(p.y(), position.y(), sizes.height());
        }

        static constexpr bool contains(pxside_t e_pos, pxside_t w_pos, pxside_t w_size) noexcept
        {
            return e_pos >= w_pos && e_pos < (w_size + w_pos);
        }
    };
}