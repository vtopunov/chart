#pragma once

#include <px/fwd.h>

namespace widget
{
    struct rectangle
    {
        px::point2d position{};
        px::size2d sizes{};

        template<class Point>
        constexpr bool contains(const Point& p) const noexcept
        {
            using position_on_axis_t = std::decay_t<decltype(p.x())>;
            static_assert(std::is_same_v<position_on_axis_t, std::decay_t<decltype(p.y())>>);

            constexpr auto contains1d = [](position_on_axis_t p, pxside_t p0, pxside_t dp) noexcept
            {
                return p >= p0 && p < (p0 + dp);
            };

            return contains1d(p.x(), position.x(), sizes.width())
                && contains1d(p.y(), position.y(), sizes.height());
        }
    };
}