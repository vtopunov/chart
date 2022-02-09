#pragma once

#include <cstddef>

#include <core/assert.h>
#include <core/limits.h>

constexpr size_t operator "" _uz(unsigned long long value) noexcept
{
    return value;
}


template<size_t mul> [[nodiscard]]
constexpr size_t size_mul(size_t size) noexcept
{
    static_assert(mul > 0_uz);

    [[maybe_unused]]
    constexpr size_t overflow = numeric_max_v<size_t> / mul;

    D_ASSERT(size <= overflow);

    return size * mul;
}

template<size_t align> [[nodiscard]]
constexpr size_t size_align(size_t size) noexcept
{
    static_assert(align > 0_uz);

    constexpr auto rem = align - 1_uz;
    static_assert((align & rem) == 0_uz);

    constexpr auto mask = ~rem;
    return (size + rem) & mask;
}



