#pragma once

#include <core/rectangle.h>

namespace px
{
    using pxside_t = unsigned int;
    using pxoff_t = int32_t;
    static_assert(sizeof(pxoff_t) >= sizeof(pxside_t));

    using point2d = ::point2d<pxside_t>;
    using off2d = ::point2d<pxoff_t>;
    using size2d = ::size2d<pxside_t>;
    
    using rectangle = ::rectangle<pxside_t>;


    namespace literals
    {
        [[nodiscard]]
        constexpr pxside_t operator"" _px(unsigned long long side) noexcept
        {
            return narrow_cast<pxside_t>(side);
        }
    }
}

namespace px_literals
{
    using namespace px::literals;
}

using px::pxside_t;
using px::pxoff_t;

using namespace px_literals;