#pragma once

#include <type_traits>
#include <limits>

#include <core/assert.h>

#undef min
#undef max

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow

template<class Target, class Source>
constexpr bool is_narrowing_v = std::numeric_limits<Target>::digits < std::numeric_limits<Source>::digits;

template<class Target, class Source>
constexpr bool is_signed_unsigned_v = std::is_signed_v<Source> && std::is_unsigned_v<Target>;

template<class Target, class Source>
constexpr bool is_integrals_v = std::is_integral_v<Source> && std::is_integral_v<Target>;

template<class T, class S>
constexpr bool is_safe_integral_conversion_v = is_integrals_v<T, S> && !is_narrowing_v<T, S> && !is_signed_unsigned_v<T, S>;

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t
<
    is_integrals_v<Target, Source>,
    bool
>
is_safe_upper_narrowing_conversion(Source v) noexcept
{
    if constexpr (is_narrowing_v<Target, Source>)
    {
        constexpr auto upper = static_cast<Source>(std::numeric_limits<Target>::max());
        return v <= upper;
    }
    else
    {
        return true;
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t
<
    is_integrals_v<Target, Source>,
    bool
>
is_safe_lower_narrowing_conversion(Source v) noexcept
{
    if constexpr (std::is_signed_v<Source>)
    {
        if constexpr (std::is_unsigned_v<Target> || is_narrowing_v<Target, Source>)
        {
            constexpr auto lowest = static_cast<Source>(std::numeric_limits<Target>::lowest());
            return v >= lowest;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t
<
    is_integrals_v<Target, Source>,
    bool
>
is_safe_narrowing_conversion(Source v) noexcept
{
    return is_safe_upper_narrowing_conversion<Target>(v)
        && is_safe_lower_narrowing_conversion<Target>(v);
}

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t
<
    is_integrals_v<Target, Source>,
    Target
>
narrow_cast(Source v) noexcept
{
    D_ASSERT(is_safe_narrowing_conversion<Target>(v));
    return static_cast<Target>(v);
}

template<class T> [[nodiscard]]
constexpr std::enable_if_t
<
    std::is_integral_v<T>,
    std::make_unsigned_t<T>
>
to_unsingned(T signed_value) noexcept
{
    return narrow_cast<std::make_unsigned_t<T>>(signed_value);
}

template<class T> [[nodiscard]]
constexpr std::enable_if_t
<
    std::is_integral_v<T>,
    std::make_signed_t<T>
>
to_singned(T unsigned_value) noexcept
{
    return narrow_cast<std::make_signed_t<T>>(unsigned_value);
}

#pragma warning(pop)
