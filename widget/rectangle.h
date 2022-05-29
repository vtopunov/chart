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
            constexpr auto contains1d = [](pxside_t p, pxside_t p0, pxside_t dp) noexcept
            {
                return p >= p0 && p < (p0 + dp);
            };

            return contains1d(p.x(), position.x(), sizes.width())
                && contains1d(p.y(), position.y(), sizes.height());
        }
    };
}