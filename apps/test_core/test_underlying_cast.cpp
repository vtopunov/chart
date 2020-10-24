#include <cstdio>
#include <cstdint>

#include <core/underlying_cast.h>

#include <core/assert.h>

void test_underlying_cast() noexcept
{
    enum class u16_enum : uint16_t
    {};

    enum class i16_enum : int16_t
    {};

    enum class u32_enum : uint32_t
    {};

    enum class i32_enum : int32_t
    {};

    static_assert( is_safe_underlying_conversion_v<uint16_t, u16_enum> );
    static_assert( !is_safe_underlying_conversion_v<int16_t, u16_enum> );
    static_assert( !is_safe_underlying_conversion_v<uint16_t, i16_enum> );
    static_assert( is_safe_underlying_conversion_v<int16_t, i16_enum> );

    static_assert( is_safe_underlying_conversion_v<uint32_t, u16_enum> );
    static_assert( is_safe_underlying_conversion_v<int32_t, u16_enum> );
    static_assert( !is_safe_underlying_conversion_v<uint32_t, i16_enum> );
    static_assert( is_safe_underlying_conversion_v<int32_t, i16_enum> );

    static_assert( !is_safe_underlying_conversion_v<uint16_t, u32_enum> );
    static_assert( !is_safe_underlying_conversion_v<int16_t, u32_enum> );
    static_assert( !is_safe_underlying_conversion_v<uint16_t, i32_enum> );
    static_assert( !is_safe_underlying_conversion_v<int16_t, i32_enum> );

    static_assert( is_safe_underlying_conversion_v<uint32_t, u32_enum> );
    static_assert( !is_safe_underlying_conversion_v<int32_t, u32_enum> );
    static_assert( !is_safe_underlying_conversion_v<uint32_t, i32_enum> );
    static_assert( is_safe_underlying_conversion_v<int32_t, i32_enum> );

    static_assert( is_safe_underlying_conversion_v<uint64_t, u32_enum> );
    static_assert( is_safe_underlying_conversion_v<int64_t, u32_enum> );
    static_assert( !is_safe_underlying_conversion_v<uint64_t, i32_enum> );
    static_assert( is_safe_underlying_conversion_v<int64_t, i32_enum> );

    constexpr auto u16_e = underlying_cast<u16_enum>( uint16_t{ 0 } );
    constexpr auto u32_e = underlying_cast<u32_enum>( u16_e );
    constexpr auto i64 = underlying_cast<int64_t>( u32_e );
    constexpr auto i16 = narrow_cast<int16_t>( i64 );
    constexpr auto i16_e = underlying_cast<i16_enum>( i16 );
    static_assert( !underlying_cast<int16_t>( i16_e ) );

    D_ASSERT( !errno );
}