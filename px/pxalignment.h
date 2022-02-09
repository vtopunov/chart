#pragma once

#include <type_traits>

#include <core/size_type.h>

namespace px
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