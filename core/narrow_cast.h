#pragma once

#include <type_traits>
#include <limits>

#include <core/utility.h>

#undef min
#undef max

#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow

template <class E> [[nodiscard]]
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

template <bool, class T>
struct underlying_type_if
{
    using type = T;
};

template <class T>
struct underlying_type_if<true, T>
{
    using type = std::underlying_type_t<T>;
};

template <bool condition, class T>
using underlying_type_if_t = typename underlying_type_if<condition, T>::type;

template <class T>
struct remove_enum
{
    using type = underlying_type_if_t<std::is_enum_v<T>, T>;
};

template <class T>
using remove_enum_t = typename remove_enum<T>::type;

namespace private_detail_narrow_cast
{
    template<class Target, class Source>
    constexpr bool is_narrowing_v = std::numeric_limits<Target>::digits < std::numeric_limits<Source>::digits;

    template<class Target, class Source>
    constexpr bool is_signed2unsigned_v = std::is_signed_v<Source> && std::is_unsigned_v<Target>;

    template<class Target, class Source>
    constexpr bool is_float2int_v = std::is_floating_point_v<Source> && std::is_integral_v<Target>;

    template<class T, class S>
    constexpr bool is_safe_numeric_conversion1_v = std::is_arithmetic_v<T> && std::is_arithmetic_v<S> && !is_narrowing_v<T, S> && !is_signed2unsigned_v<T, S> && !is_float2int_v<T, S>;

    template<class T, class S>
    constexpr bool is_enum2enum_v = std::is_enum_v<T> && std::is_enum_v<S>;

    template<class T, class S>
    constexpr bool is_safe_numeric_conversion0_v = std::is_same_v<T, S> || (!is_enum2enum_v<T, S> && is_safe_numeric_conversion1_v<remove_enum_t<T>, remove_enum_t<S>>);

    template<class T, class S>
    constexpr bool is_safe_numeric_conversion_v = is_safe_numeric_conversion0_v<std::remove_cv_t<T>, std::remove_cv_t<S>>;

    template<class Target, class Source>
    [[nodiscard]] constexpr bool is_safe_upper_narrowing_conversion(const Source& v) noexcept
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

    template<class Target, class Source>
    [[nodiscard]] constexpr bool is_safe_lower_narrowing_conversion(const Source& v) noexcept
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

    template<class Target, class Source>
    [[nodiscard]] constexpr bool is_safe_integral_to_floating_point_conversion(const Source& v) noexcept
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


    template<class Target, class Source>
    [[nodiscard]] constexpr bool is_safe_narrowing_conversion(const Source& v) noexcept
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
            return is_safe_numeric_conversion_v<Target, Source>;
        }
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target narrow_cast0(const Source& v) noexcept
    {
        if constexpr (is_safe_numeric_conversion_v<Target, Source>)
        {
            return v;
        }
        else
        {
            D_ASSERT(is_safe_narrowing_conversion<Target>(v));
            return static_cast<Target>(v);
        }
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target narrow_cast(const Source& v) noexcept
    {
        if constexpr (std::is_enum_v<Source>)
        {
            static_assert(!std::is_enum_v<Target>);
            return narrow_cast0<Target>(to_underlying(v));
        }
        else if constexpr (std::is_enum_v<Target>)
        {
            using target_underlying_t = std::underlying_type_t<Target>;
            return static_cast<Target>(narrow_cast0<target_underlying_t>(v));
        }
        else
        {
            return narrow_cast0<Target>(v);
        }
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target safe_numeric_cast(const Source& v) noexcept
    {
        static_assert(is_safe_numeric_conversion_v<Target, Source>);

        if constexpr (std::is_enum_v<Source>)
        {
            static_assert(!std::is_enum_v<Target>);
            return to_underlying(v);
        }
        else if constexpr (std::is_enum_v<Target>)
        {
            using target_underlying_t = std::underlying_type_t<Target>;
            return static_cast<Target>(static_cast<target_underlying_t>(v));
        }
        else
        {
            return v;
        }
    }
}

using private_detail_narrow_cast::is_narrowing_v;
using private_detail_narrow_cast::is_safe_numeric_conversion_v;
using private_detail_narrow_cast::is_safe_narrowing_conversion;
using private_detail_narrow_cast::narrow_cast;
using private_detail_narrow_cast::safe_numeric_cast;

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
    std::make_unsigned_t<T>
>
clamp_to_unsingned(T signed_value) noexcept
{
    if constexpr (std::is_signed_v<T>)
    {
        constexpr T zero{};
        return static_cast<std::make_unsigned_t<T>>((signed_value < zero) ? zero : signed_value);
    }
    else
    {
        return signed_value;
    }
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
