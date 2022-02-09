#pragma once

#include <core/warnings.h>
#include <core/assert.h>
#include <core/limits.h>
#include <core/type_traits.h>
#include <core/zero.h>

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

template <class E> [[nodiscard]]
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


namespace private_detail_narrow
{
    template<class Target, class Source>
    constexpr bool is_narrowing_v = std::numeric_limits<Target>::digits < std::numeric_limits<Source>::digits;

    template<class Target, class Source>
    using is_narrowing = std::bool_constant<is_narrowing_v<Target, Source>>;

    template<class Target, class Source>
    constexpr bool is_narrowing_or_same_v = std::numeric_limits<Target>::digits <= std::numeric_limits<Source>::digits;

    template<class Target, class Source>
    using is_signed2unsigned = std::conjunction<
        std::is_signed<Source>,
        std::is_unsigned<Target>
    >;

    template<class Target, class Source>
    using is_float2int = std::conjunction<
        std::is_floating_point<Source>,
        std::is_integral<Target>
    >;

    template<class T, class S>
    using is_enum2enum = std::conjunction<
        std::is_enum<T>,
        std::is_enum<S>
    >;

    template<class T, class S>
    using is_safe_numeric_conversion2 = std::conjunction<
        std::is_arithmetic<T>,
        std::is_arithmetic<S>,
        std::negation<is_narrowing<T, S>>,
        std::negation<is_signed2unsigned<T, S>>,
        std::negation<is_float2int<T, S>>
    >;

    template<class T, class S>
    using is_safe_numeric_conversion1 = std::conjunction<
        std::negation<is_enum2enum<T, S>>,
        is_safe_numeric_conversion2<remove_enum_t<T>, remove_enum_t<S>>
    >;

    template<class T, class S>
    constexpr bool is_safe_numeric_conversion0_v = std::disjunction_v<
        std::is_same<T, S>,
        is_safe_numeric_conversion1<T, S>
    >;

    template<class T, class S>
    constexpr bool is_safe_numeric_not_same_conversion0_v = std::conjunction_v<
        std::negation<std::is_same<T, S>>,
        is_safe_numeric_conversion1<T, S>
    >;

    template<class T, class S>
    constexpr bool is_safe_numeric_conversion_v 
        = is_safe_numeric_conversion0_v<std::remove_cv_t<T>, std::remove_cv_t<S>>;

    template<class T, class S>
    constexpr bool is_safe_numeric_not_same_conversion_v
        = is_safe_numeric_not_same_conversion0_v<std::remove_cv_t<T>, std::remove_cv_t<S>>;

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

using private_detail_narrow::is_narrowing_v;
using private_detail_narrow::is_narrowing_or_same_v;
using private_detail_narrow::is_safe_numeric_conversion_v;
using private_detail_narrow::is_safe_numeric_not_same_conversion_v;
using private_detail_narrow::is_safe_narrowing_conversion;
using private_detail_narrow::narrow_cast;
using private_detail_narrow::safe_numeric_cast;

D_WARNING_POP
