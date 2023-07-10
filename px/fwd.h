#pragma once

#include <core/rectangle.h>


namespace px
{
    using pxside_t = unsigned int;
    using pxoff_t = int32_t;
    static_assert(sizeof(pxoff_t) >= sizeof(pxside_t));

    using pxvec2 = ::vec2<pxside_t>;
    using pxpoint2d = ::point2d<pxside_t>;
    using pxoff2d = ::point2d<pxoff_t>;
    using pxsize2d = ::size2d<pxside_t>;
    
    using pxrectangle = ::rectangle<pxside_t>;

    namespace literals
    {
        [[nodiscard]]
        constexpr pxside_t operator"" _px(unsigned long long side) noexcept
        {
            return narrow_cast<pxside_t>(side);
        }

        [[nodiscard]]
        constexpr pxoff_t operator"" _pxz(unsigned long long side) noexcept
        {
            return narrow_cast<pxoff_t>(side);
        }
    }
}

namespace px_literals
{
    using namespace px::literals;
}

using px::pxside_t;
using px::pxoff_t;
using px::pxvec2;
using px::pxpoint2d;
using px::pxoff2d;
using px::pxsize2d;
using px::pxrectangle;

using namespace px_literals;