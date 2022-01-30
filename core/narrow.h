#pragma once

#include <type_traits>
#include <limits>

#include <core/utility.h>

#undef min
#undef max

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast)

using doublemax_t = long double;

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

namespace private_detail_narrow
{
    template<class Target, class Source>
    constexpr bool is_narrowing_v = std::numeric_limits<Target>::digits < std::numeric_limits<Source>::digits;

    template<class Target, class Source>
    constexpr bool is_narrowing_or_same_v = std::numeric_limits<Target>::digits <= std::numeric_limits<Source>::digits;

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

    template<class T>
    using signed_int0_t = std::conditional_t<
        is_narrowing_or_same_v<T, ptrdiff_t>, ptrdiff_t,
        std::conditional_t<is_narrowing_or_same_v<T, int64_t>, int64_t, intmax_t>
    >;

    template<class T>
    using signed_t = std::conditional_t<std::is_integral_v<T>, copy_const_t<T, signed_int0_t<std::remove_cv_t<T>>>, T>;

    template<class T>
    [[nodiscard]] constexpr signed_t<T> to_signed(T value) noexcept
    {
        return narrow_cast<signed_t<T>>(value);
    }

    template<class T>
    using fp0_t = std::conditional_t<is_narrowing_or_same_v<T, double_t>, double_t, doublemax_t>;

    template<class T>
    using fp_t = copy_const_t<T, fp0_t<std::remove_cv_t<T>>>;

    template<class T>
    [[nodiscard]] constexpr fp_t<T> to_fp(T value) noexcept
    {
        return value;
    }

    template<class T>
    using far_unsigned0_t = std::conditional_t<
        is_narrowing_v<T, size_t>, size_t,
        std::conditional_t<is_narrowing_v<T, uint64_t>, uint64_t, uintmax_t>
    >;

    template<class T>
    using far_signed0_t = std::conditional_t<
        is_narrowing_v<T, ptrdiff_t>, ptrdiff_t,
        std::conditional_t<is_narrowing_v<T, int64_t>, int64_t, intmax_t>
    >;

    template<class T>
    using far_fp0_t = std::conditional_t<is_narrowing_v<T, double_t>, double_t, doublemax_t>;

    template<class T>
    using far_unsigned_t = copy_const_t<T, far_unsigned0_t<std::remove_cv_t<T>>>;

    template<class T>
    using far_signed_t = copy_const_t<T, far_signed0_t<std::remove_cv_t<T>>>;

    template<class T>
    using far_int_t = std::conditional_t<std::is_unsigned_v<T>, far_unsigned_t<T>, far_signed_t<T>>;

    template<class T>
    using far_fp_t = copy_const_t<T, far_fp0_t<std::remove_cv_t<T>>>;

    template<class T>
    using far_t = std::conditional_t<std::is_integral_v<T>, far_int_t<T>, std::conditional_t<std::is_floating_point_v<T>, far_fp_t<T>, T>>;

    template<class T>
    [[nodiscard]] constexpr far_t<T> to_far(T value) noexcept
    {
        return value;
    }
}

using private_detail_narrow::is_narrowing_v;
using private_detail_narrow::is_narrowing_or_same_v;
using private_detail_narrow::is_safe_numeric_conversion_v;
using private_detail_narrow::is_safe_narrowing_conversion;
using private_detail_narrow::narrow_cast;
using private_detail_narrow::safe_numeric_cast;
using private_detail_narrow::signed_t;
using private_detail_narrow::to_signed;
using private_detail_narrow::fp_t;
using private_detail_narrow::to_fp;
using private_detail_narrow::far_t;
using private_detail_narrow::to_far;

D_WARNING_POP
