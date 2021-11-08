#pragma once

#include <type_traits>
#include <limits>

#include <core/utility.h>

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
is_safe_upper_narrowing_conversion(const Source& v) noexcept
{
    if constexpr (is_narrowing_v<Target, Source>)
    {
        constexpr auto upper = static_cast<Source>(numeric_max_v<Target>);
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
is_safe_lower_narrowing_conversion(const Source& v) noexcept
{
    if constexpr (std::is_signed_v<Source>)
    {
        if constexpr (std::is_unsigned_v<Target> || is_narrowing_v<Target, Source>)
        {
            constexpr auto lowest = static_cast<Source>(numeric_lowest_v<Target>);
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
    std::is_floating_point_v<Target> && std::is_integral_v<Source>,
    bool
>
is_safe_integral_to_floating_point_conversion(const Source& v) noexcept
{
    constexpr auto target_digits = std::numeric_limits<Target>::digits;
    constexpr auto source_digits = std::numeric_limits<Source>::digits;

    if constexpr (target_digits < source_digits)
    {
        constexpr auto upper_source = numeric_max_v<Source>;
        constexpr auto max_mantissa = upper_source >> (source_digits - target_digits);
        return constexpr_abs(v) <= max_mantissa;
    }
    else
    {
        return true;
    }
}


template<class Target, class Source> [[nodiscard]]
constexpr bool is_safe_narrowing_conversion(const Source& v) noexcept
{
    if constexpr (std::is_integral_v<Source>)
    {
        if constexpr (std::is_integral_v<Target>)
        {
            return is_safe_upper_narrowing_conversion<Target>(v)
                && is_safe_lower_narrowing_conversion<Target>(v);
        }
        else if constexpr (std::is_floating_point_v<Target>)
        {
            return is_safe_integral_to_floating_point_conversion<Target>(v);
        }
        else
        {
            return false;
        }
    }
    else
    {
        D_UNUSED(v);

        if constexpr (std::is_floating_point_v<Source> && std::is_floating_point_v<Target>)
        {
            return !is_narrowing_v<Target, Source>;
        }
        else
        {
            return std::is_same_v<std::remove_cv_t<Source>, std::remove_cv_t<Target>>;
        }
    }
}

template<class Target, class Source> [[nodiscard]]
constexpr Target narrow_cast(const Source& v) noexcept
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
