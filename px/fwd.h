#pragma once

#include <core/rect.h>

namespace px
{
    using pxside_t = unsigned int;

    using vec2 = ::vec2<pxside_t>;
    using point2d = ::point2d<pxside_t>;
    using size2d = ::size2d<pxside_t>;
    using rect = ::rect<pxside_t>;

    template<class T> [[nodiscard]]
    constexpr pxside_t as_pxside(const T& value) noexcept
    {
        return narrow_cast<pxside_t>(value);
    }

    namespace literals
    {
        [[nodiscard]]
        constexpr pxside_t operator"" _px(unsigned long long side) noexcept
        {
            return as_pxside(side);
        }
    }
}

namespace px_literals
{
    using namespace px::literals;
}

using px::pxside_t;
using px::as_pxside;

using namespace px_literals;