#pragma once

#include <core/rectangle.h>


namespace px
{
    using pix8_t = byte_tint_t;
    static_assert(1u == sizeof(pix8_t));

    using pxsize_t = uint32_t;
    using pxoff_t = int32_t;
    static_assert(sizeof(pxoff_t) >= sizeof(pxsize_t));

    using lpxsize_t = uint64_t;
    using lpxoff_t = int64_t;
    static_assert(sizeof(lpxsize_t) > sizeof(pxsize_t));
    static_assert(sizeof(lpxoff_t) > sizeof(pxoff_t));
    static_assert(sizeof(lpxoff_t) >= sizeof(lpxsize_t));

    using pxvec2 = vec2<pxsize_t>;
    using pxpoint2d = point2d<pxsize_t>;
    using pxoff2d = point2d<pxoff_t>;
    using pxsize2d = size2d<pxsize_t>;

    using lpxoff2d = point2d<lpxoff_t>;

    using pxrectangle = rectangle<pxsize_t>;
    using pxzrectangle = rectangle<pxoff_t>;

    using real_t = double_t;
    using real_vec2 = vec2<real_t>;
    using real_point2d = point2d<real_t>;
    using real_size2d = size2d<real_t>;
    using real_point2d_cspan = span<const real_point2d>;


    namespace literals
    {
        [[nodiscard]]
        constexpr pxsize_t operator"" _npx(unsigned long long side) noexcept
        {
            return narrow<pxsize_t>(side);
        }

        [[nodiscard]]
        constexpr pxoff_t operator"" _npxz(unsigned long long side) noexcept
        {
            return narrow<pxoff_t>(static_cast<long long>(side));
        }

        [[nodiscard]]
        constexpr lpxsize_t operator"" _npxl(unsigned long long side) noexcept
        {
            static_assert(std::is_same_v<lpxsize_t, decltype(side)>);
            return side;
        }

        [[nodiscard]]
        constexpr lpxoff_t operator"" _npxlz(unsigned long long value) noexcept
        {
            static_assert(std::is_same_v<lpxoff_t, std::make_signed_t<decltype(value)>>);
            return static_cast<lpxoff_t>(value);
        }
    }
}

namespace px_literals
{
    using namespace px::literals;
}

using px::pix8_t;
using px::pxsize_t;
using px::pxoff_t;
using px::lpxoff_t;
using px::pxvec2;
using px::pxpoint2d;
using px::pxoff2d;
using px::lpxoff2d;
using px::pxsize2d;
using px::pxrectangle;
using px::pxzrectangle;

using namespace px_literals;