#pragma once

#include <type_traits>
#include <limits>

#include <core/assert.h>

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow

template<class Target, class Source>
constexpr bool is_narrowing_v = std::numeric_limits<Target>::digits < std::numeric_limits<Source>::digits;

template<class Target, class Source>
constexpr bool is_signed_unsigned_v = std::is_signed_v<Source> && std::is_unsigned_v<Target>;

template<class Target, class Source>
constexpr bool is_signed_signed_v = std::is_signed_v<Source> && std::is_signed_v<Target>;

template<class Target, class Source>
constexpr Source integer_overflow_bound_v = Source{ 1 } << std::numeric_limits<Target>::digits;

template<class Target, class Source>
constexpr std::enable_if_t<is_narrowing_v<Target, Source> && is_signed_signed_v<Target, Source>, bool> is_safe_narrowing_conversion( Source v ) noexcept
{
    constexpr Source bound{ integer_overflow_bound_v<Target, Source> };
    return v < bound && v >= -bound;
}

template<class Target, class Source>
constexpr std::enable_if_t<is_narrowing_v<Target, Source> && is_signed_unsigned_v<Target, Source>, bool> is_safe_narrowing_conversion( Source v ) noexcept
{
    return v < integer_overflow_bound_v<Target, Source> && v >= Source{ 0 };
}

template<class Target, class Source>
constexpr std::enable_if_t<is_narrowing_v<Target, Source> && std::is_unsigned_v<Source>, bool> is_safe_narrowing_conversion( Source v ) noexcept
{
    return v < integer_overflow_bound_v<Target, Source>;
}

template<class Target, class Source>
constexpr std::enable_if_t<!is_narrowing_v<Target, Source> && is_signed_unsigned_v<Target, Source>, bool> is_safe_narrowing_conversion( Source v ) noexcept
{
    return v >= Source{ 0 };
}

template<class Target, class Source>
constexpr bool is_safe_integral_conversion_v = !is_narrowing_v<Target, Source> && !is_signed_unsigned_v<Target, Source>;

template<class Target, class Source>
constexpr std::enable_if_t<is_safe_integral_conversion_v<Target, Source>, bool> is_safe_narrowing_conversion(Source) noexcept
{
    return true;
}

template<class Target, class Source>
constexpr Target narrow_cast( Source v ) noexcept
{
    assert( is_safe_narrowing_conversion<Target>( v ) );
    return static_cast<Target>( v );
}

#pragma warning(pop)
