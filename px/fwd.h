#pragma once

#include <core/rectangle.h>


namespace px
{
    using npx_t = uint32_t;
    using pxoff_t = int32_t;
    static_assert(sizeof(pxoff_t) >= sizeof(npx_t));

    using pxvec2d = vec2<npx_t>;
    using pxpoint2d = point2d<npx_t>;
    using pxoff2d = point2d<pxoff_t>;
    using pxsize2d = size2d<npx_t>;

    using pxrectangle = rectangle<npx_t>;

    using real_t = double_t;
    using real_vec2 = vec2<real_t>;
    using real_point2d = point2d<real_t>;
    using real_size2d = size2d<real_t>;
    using real_point2d_cspan = span<const real_point2d>;

    namespace literals
    {
        [[nodiscard]]
        constexpr npx_t operator"" _npx(unsigned long long side) noexcept
        {
            return narrow<npx_t>(side);
        }

        [[nodiscard]]
        constexpr pxoff_t operator"" _pxoff(unsigned long long side) noexcept
        {
            return narrow<pxoff_t>(static_cast<long long>(side));
        }
    }

    using namespace literals;

    constexpr pxsize2d no_sizes{ 0_npx, 0_npx };
    static_assert(!no_sizes.has_positive_mark());
    static_assert(!no_sizes.has_positive_square());
}

namespace px_literals
{
    using namespace px::literals;
}

using px::npx_t;
using px::pxoff_t;
using px::pxvec2d;
using px::pxpoint2d;
using px::pxoff2d;
using px::pxsize2d;
using px::pxrectangle;

using namespace px_literals;