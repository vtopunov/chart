#pragma once

#include <core/size_type.h>
#include <core/rectangle.h>


namespace px
{
    using pxside_t = uint32_t;
    using pxoff_t = int32_t;
    static_assert(sizeof(pxoff_t) >= sizeof(pxside_t));

    using lpxside_t = uint64_t;
    using lpxoff_t = int64_t;
    static_assert(sizeof(lpxside_t) > sizeof(pxside_t));
    static_assert(sizeof(lpxoff_t) > sizeof(pxoff_t));
    static_assert(sizeof(lpxoff_t) >= sizeof(lpxside_t));

    using pxvec2 = ::vec2<pxside_t>;
    using pxpoint2d = ::point2d<pxside_t>;
    using pxoff2d = ::point2d<pxoff_t>;
    using pxsize2d = ::size2d<pxside_t>;

    using lpxoff2d = ::point2d<lpxoff_t>;

    using pxrectangle = ::rectangle<pxside_t>;
    using pxzrectangle = ::rectangle<pxoff_t>;

    using real_t = double_t;
    using real_vec2 = vec2<real_t>;
    using real_point2d = point2d<real_t>;
    using real_size2d = size2d<real_t>;
    using real_point2d_cspan = span<const real_point2d>;


    namespace alignment_implementation
    {
        constexpr size_t default_alignment{ 4_uz };
        constexpr size_t dynamic_alignment{ numeric_max_v<size_t> };

        template<size_t Alignment>
        struct is_dynamic_alignment : std::bool_constant<Alignment == dynamic_alignment>
        {};

        template<size_t Alignment>
        constexpr bool is_dynamic_alignment_v = is_dynamic_alignment<Alignment>::value;

        template<size_t PxSize, size_t Alignment>
        [[nodiscard]] constexpr size_t aligned_width(size_t width) noexcept
        {
            constexpr auto px_size = PxSize;
            constexpr auto alignment = Alignment;

            if constexpr (px_size < alignment)
            {
                static_assert((alignment % px_size) == 0_uz);

                constexpr auto size_line_alignment = alignment / px_size;
                return size_align< size_line_alignment >(width);
            }
            else
            {
                static_assert((px_size % alignment) == 0_uz);
                return width;
            }
        }
    }

    using namespace alignment_implementation;


    namespace literals
    {
        [[nodiscard]]
        constexpr pxside_t operator"" _px(unsigned long long side) noexcept
        {
            return narrow<pxside_t>(side);
        }

        [[nodiscard]]
        constexpr pxoff_t operator"" _pxz(unsigned long long side) noexcept
        {
            return narrow<pxoff_t>(static_cast<long long>(side));
        }

        [[nodiscard]]
        constexpr lpxside_t operator"" _lpx(unsigned long long side) noexcept
        {
            static_assert(std::is_same_v<lpxside_t, decltype(side)>);
            return side;
        }

        [[nodiscard]]
        constexpr lpxoff_t operator"" _lpxz(unsigned long long value) noexcept
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

using px::pxside_t;
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