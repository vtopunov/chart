#pragma once

#include <core/utility.h>

namespace px
{
    inline constexpr size_t default_alignment{ 4_uz };
    inline constexpr size_t dynamic_alignment{ numeric_max_v<size_t> };

    template<size_t Alignment>
    inline constexpr bool is_dynamic_alignment_v = (Alignment == dynamic_alignment);


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