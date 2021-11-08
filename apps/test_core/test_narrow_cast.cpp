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
    static_assert(is_safe_narrowing_conversion<double>(numeric_max_v<float>));
    static_assert(is_safe_narrowing_conversion<double>(numeric_max_v<double>));
    static_assert(is_safe_narrowing_conversion<float>(numeric_max_v<float>));
    static_assert(!is_safe_narrowing_conversion<float>(numeric_max_v<double>));


    constexpr uint32_t max_mantissa = uint32_t(-1) >> 8;
    D_ASSERT(is_safe_narrowing_conversion<float>(max_mantissa));
    
    constexpr int32_t max_mantissa_i = static_cast<int32_t>(max_mantissa);
    D_ASSERT(is_safe_narrowing_conversion<float>(max_mantissa_i));

    {
        auto f = static_cast<float>(max_mantissa);
        for (auto ui = max_mantissa; (max_mantissa - ui) < 0xffu; --ui, f = f - 1.0f)
        {
            auto uif = static_cast<uint32_t>(f);
            D_ASSERT(uif == ui);
        }
    }

    static_assert(!is_safe_narrowing_conversion<float>(max_mantissa+1));


    D_ASSERT( !errno );
}