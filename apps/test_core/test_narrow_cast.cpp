#include <cstdio>
#include <cstdint>

#include <core/narrow_cast.h>
#include <core/assert.h>

void test_narrow_cast() noexcept
{
    static_assert( !is_safe_integral_conversion_v<uint32_t, int32_t> );
    static_assert( !is_safe_integral_conversion_v<int32_t, uint32_t> );
    static_assert( is_safe_integral_conversion_v<int32_t, int32_t> );
    static_assert( is_safe_integral_conversion_v<uint32_t, uint32_t> );

    static_assert( !is_safe_integral_conversion_v<uint16_t, int32_t> );
    static_assert( !is_safe_integral_conversion_v<uint16_t, uint32_t> );
    static_assert( !is_safe_integral_conversion_v<int16_t, int32_t> );
    static_assert( !is_safe_integral_conversion_v<int16_t, uint32_t> );

    static_assert( !is_safe_integral_conversion_v<uint32_t, int16_t> );
    static_assert( is_safe_integral_conversion_v<uint32_t, uint16_t> );
    static_assert( is_safe_integral_conversion_v<int32_t, int16_t> );
    static_assert( is_safe_integral_conversion_v<int32_t, uint16_t> );

    static_assert( !is_safe_narrowing_conversion<uint32_t>( -1L ) );
    static_assert( !is_safe_narrowing_conversion<int16_t>( -0x8001L ) );
    static_assert( !is_safe_narrowing_conversion<int16_t>( 0x8000L ) );
    static_assert( !is_safe_narrowing_conversion<uint16_t>( -1L ) );
    static_assert( !is_safe_narrowing_conversion<uint16_t>( 0x10000L ) );
    static_assert( !is_safe_narrowing_conversion<int32_t>( 0xffffffffUL ) );

    D_ASSERT( !errno );
}