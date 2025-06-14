#pragma once

#include <bit>
#include <compare>
#include <numeric>

#include <core/span.h>


static_assert(std::is_unsigned_v<denomi_t>);

template<denomi_t UDen>
constexpr bool is_dynamic_denominator_v = (UDen == dynamic_denominator);

template<denomi_t UDen>
using is_dynamic_denominator = std::bool_constant< is_dynamic_denominator_v<UDen> >;

template<denomi_t UDen>
struct denominator_traits
{
    static constexpr bool has_single_bit{ std::has_single_bit(UDen) };
    static constexpr int fraction_width{ std::bit_width(UDen) - 1 };
    static constexpr bool is_dynamic = is_dynamic_denominator_v<UDen>;
};

static_assert(!denominator_traits<dynamic_denominator>::has_single_bit);
static_assert(-1 == denominator_traits<dynamic_denominator>::fraction_width);


template<denomi_t UDen, class T>
[[nodiscard]] constexpr bool has_denominator_mul(const T num) noexcept
{
    static_assert(UDen > 0u);
    static_assert(std::is_integral_v<T>);
    static_assert(sizeof(T) <= sizeof(denomi_t));

    if constexpr (std::is_signed_v<T>)
    {
        using numden_common_t = std::make_signed_t<denomi_t>;
        constexpr auto numden_common_maxi = numeric_max_v<numden_common_t>;
        constexpr auto numden_common_mini = numeric_min_v<numden_common_t>;
        constexpr auto den = numeric_cast<numden_common_t>(UDen);
        constexpr auto overflow_maxi = numden_common_maxi / den;
        constexpr auto overflow_mini = numden_common_mini / den;
        return (num >= overflow_mini) && (num <= overflow_maxi);
    }
    else
    {
        constexpr auto u_denominator_maxi = numeric_max_v<denomi_t>;
        constexpr auto overflow = u_denominator_maxi / UDen;
        return num <= overflow;
    }
}

template<class T>
using denominator_for_t = copy_signed_t<T, denomi_t>;

template<denomi_t UDen, class T>
[[nodiscard]] constexpr denominator_for_t<T> denominator_mul(const T num) noexcept
{
    D_ASSERT_OR_ASSUME(has_denominator_mul<UDen>(num));
    constexpr auto den = numeric_cast<denominator_for_t<T>>(UDen);
    return num * den;
}

namespace private_detail_rational
{
    template <class T, denomi_t UDen>
    struct rational_numden_base
    {
        using int_type = T;
        static_assert(std::is_integral_v<int_type>);

        int_type num;
        static_assert(is_safe_narrowing_conversion<int_type>(UDen));
        static constexpr int_type den{ static_cast<int_type>(UDen) };

        [[nodiscard]]
        constexpr auto operator<=>(const rational_numden_base&) const noexcept = default;
    };

    template <class T>
    struct rational_numden_base<T, dynamic_denominator>
    {
        using int_type = T;
        static_assert(std::is_integral_v<int_type>);

        int_type num;
        int_type den;

        [[nodiscard]]
        constexpr bool operator == (const rational_numden_base&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const rational_numden_base&) const noexcept = default;
    };

    template <class T>
    using rational_dynden_base_t = rational_numden_base<T, dynamic_denominator>;

    template<class T>
    [[nodiscard]] constexpr bool operator < (const rational_dynden_base_t<T>& left, const rational_dynden_base_t<T>& right) noexcept
    {
        const auto gcd = std::gcd(left.den, right.den);
        const auto mul_left = right.den / gcd;
        const auto mul_right = left.den / gcd;

        return (left.num * mul_left) < (right.num * mul_right);
    }

    template<class T>
    [[nodiscard]] constexpr bool operator <= (const rational_dynden_base_t<T>& left, const rational_dynden_base_t<T>& right) noexcept
    {
        return (left == right) || (left < right);
    }

    template<class T>
    [[nodiscard]] constexpr bool operator > (const rational_dynden_base_t<T>& left, const rational_dynden_base_t<T>& right) noexcept
    {
        return right < left;
    }

    template<class T>
    [[nodiscard]] constexpr bool operator >= (const rational_dynden_base_t<T>& left, const rational_dynden_base_t<T>& right) noexcept
    {
        return (left == right) || (left > right);
    }
}

template <class T, denomi_t UDen>
struct rational : private_detail_rational::rational_numden_base<T, UDen>
{
    using rational_base_type = private_detail_rational::rational_numden_base<T, UDen>;
    using typename rational_base_type::int_type;
    using rational_base_type::num;
    using rational_base_type::den;
    using denominator_traits_type = denominator_traits<UDen>;

    [[nodiscard]]
    static constexpr rational zero() noexcept
    {
        constexpr int_type zero{ zero_v<> };
        return instance(zero);
    }

    template<class Int>
    [[nodiscard]] static constexpr std::enable_if_t<std::is_integral_v<Int>, rational> instance(const Int& value) noexcept
    {
        if constexpr (denominator_traits_type::is_dynamic)
        {
            constexpr int_type one{ 1 };
            return { narrow<int_type>(value), one };
        }
        else
        {
            if constexpr (denominator_traits_type::has_single_bit)
            {
                using common_int_t = std::common_type_t<int_type, copy_signed_t<int_type, Int>>;
                return { narrow<int_type>(narrow<common_int_t>(value) << denominator_traits_type::fraction_width) };
            }
            else
            {
                return { narrow<int_type>(denominator_mul<UDen>(value)) };
            }
        }
    }

    template<class NewT>
    [[nodiscard]] static constexpr rational instance(const rational<NewT, UDen>& value) noexcept
    {
        if constexpr (denominator_traits_type::is_dynamic)
        {
            return
            {
                narrow<NewT>(value.num),
                narrow<NewT>(value.den)
            };
        }
        else
        {
            return { narrow<NewT>(value.num) };
        }
    }

    [[nodiscard]]
    static constexpr rational instance(span<const char> string) noexcept;

    [[nodiscard]]
    constexpr int_type discard_fraction() const noexcept
    {
        if constexpr (denominator_traits_type::has_single_bit)
        {
            return num >> denominator_traits_type::fraction_width;
        }
        else
        {
            return num / den;
        }
    }

    [[nodiscard]]
    constexpr int_type fraction() const noexcept
    {
        if constexpr (denominator_traits_type::has_single_bit)
        {
            constexpr int_type mask{ den - 1 };
            return num & mask;

        }
        else
        {
            return num % den;
        }
    }

    constexpr rational& operator += (const rational& right) noexcept;

    constexpr rational& operator -= (const rational& right) noexcept;


    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return !!num;
    }

    template<class U, class = std::enable_if_t<is_safe_numeric_not_same_conversion_v<U, int_type>>>
    [[nodiscard]] constexpr operator rational<U, UDen>() const noexcept
    {
        if constexpr (denominator_traits_type::is_dynamic)
        {
            return { num, den };
        }
        else
        {
            return { num };
        }
    }
};

template<class T>
rational(T, T) -> rational<T>;


template <class T>
struct rational_detector : std::false_type
{
    using removed_rational_type = T;
};

template <class T, denomi_t UDen>
struct rational_detector<rational<T, UDen>> : std::true_type
{
    using removed_rational_type = T;
};

template <class T>
struct rational_detector<const T> : rational_detector<T>
{};

template<class T>
using is_rational = rational_detector<T>;

template<class T>
struct remove_rational
{
    using type = typename rational_detector<T>::removed_rational_type;
};

template<class T>
constexpr auto is_rational_v = is_rational<T>::value;

template<class T>
using remove_rational_t = typename remove_rational<T>::type;


template<class Out, class T, denomi_t UDen>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_dynamic_denominator<UDen>>, Out> floor_to(const rational<T, UDen>& fp) noexcept
{
    return narrow<Out>(fp.discard_fraction());
}

template<class Out, class T, denomi_t UDen>
[[nodiscard]] constexpr std::enable_if_t<std::negation_v<is_dynamic_denominator<UDen>>, Out> ceil_to(const rational<T, UDen>& fp) noexcept
{
    return narrow<Out>(fp.discard_fraction() + !!fp.fraction());
}


template<class T>
[[nodiscard]] constexpr rational<T> simplify(const T& num, const T& den) noexcept
{
    D_ASSERT_OR_ASSUME(den);

    const auto gcd = std::gcd(num, den);

    return
    {
        num / gcd,
        den / gcd
    };
}

template<class T>
[[nodiscard]] constexpr rational<T> simplify(const rational<T>& value) noexcept
{
    return simplify(value.num, value.den);
}

template<class T>
[[nodiscard]] constexpr rational<T> inverse(const rational<T>& value) noexcept
{
    D_ASSERT_OR_ASSUME(value.num);

    return
    {
        value.den,
        value.num
    };
}

template<class T, denomi_t UDen>
constexpr doublemax_t rational_to_float(const rational<T, UDen>& src) noexcept 
{
    return narrow<doublemax_t>(src.num) / narrow<doublemax_t>(src.den);
}

template<class Target, class Source>
[[nodiscard]] constexpr Target rational_cast(const Source& src) noexcept
{
    if constexpr (is_rational_v<Source>)
    {
        D_ASSERT_OR_ASSUME(src.den);

        if constexpr (is_rational_v<Target>)
        {
            using int_type = typename Target::int_type;
            return
            {
                narrow<int_type>(src.num),
                narrow<int_type>(src.den)
            };
        }
        else
        {
            if constexpr (std::is_integral_v<Target>)
            {
                return narrow<Target>(src.num / src.den);
            }
            else
            {
                static_assert(std::is_floating_point_v<Target>);
                return static_cast<Target>(rational_to_float(src));
            }
        }
    }
    else
    {
        static_assert(is_rational_v<Target>);
        return Target::instance(src);
    }
}

template<class T, denomi_t UDen>
[[nodiscard]] constexpr std::enable_if_t<std::is_signed_v<T>, rational<T, UDen> > operator - (const rational<T, UDen>& left) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        return { -left.num, left.den };
    }
    else
    {
        return { -left.num };
    }
}

template<class T, uintmax_t UDen>
[[nodiscard]] constexpr rational<T, UDen> operator + (const rational<T, UDen>& left, const rational<T, UDen>& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        const auto gcd = std::gcd(left.den, right.den);
        const auto mul_left = right.den / gcd;
        const auto mul_right = left.den / gcd;

        return
        {
            left.num * mul_left + right.num * mul_right,
            mul_left * left.den
        };
    }
    else
    {
        return { left.num + right.num };
    }
}

template<class T, uintmax_t UDen>
[[nodiscard]] constexpr std::enable_if_t
<
    std::is_signed_v<T>,
    rational<T, UDen>
> operator - (const rational<T, UDen>& left, const rational<T, UDen>& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        return left + (-right);
    }
    else
    {
        return { left.num - right.num };
    }
}

template<class T, denomi_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    is_safe_numeric_conversion_v<T, U>,
    rational<T, UDen>
>
operator + (const rational<T, UDen>& left, const U& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        return
        {
            left.num + left.den * T{ right },
            left.den
        };
    }
    else
    {
        return { left + rational<T, UDen>::instance(right) };
    }
}

template<class T, uintmax_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    is_safe_numeric_conversion_v<T, U>,
    rational<T>
>
operator + (const U& left, const rational<T, UDen>& right) noexcept
{
    return right + left;
}

template<class T, uintmax_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    std::conjunction_v<is_safe_numeric_conversion<T, U>, std::is_signed<T>, std::is_signed<U>>,
    rational<T, UDen>
>
operator - (const rational<T, UDen>& left, const U& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        return
        {
            left.num - left.den * T{ right },
            left.den
        };
    }
    else
    {
        return { left - rational<T, UDen>::instance(right) };
    }
}

template<class T, uintmax_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    std::conjunction_v<is_safe_numeric_conversion<T, U>, std::is_signed<T>, std::is_signed<U>>,
    rational<T, UDen>
>
operator - (const U& left, const rational<T, UDen>& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        return
        {
            left.den * T{ left } - right.num,
            left.den
        };
    }
    else
    {
        return { rational<T, UDen>::instance(left) - right };
    }
}

template<class T, uintmax_t UDen>
constexpr rational<T, UDen>& rational<T, UDen>::operator += (const rational<T, UDen>& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        *this = *this + right;
    }
    else
    {
        num += right.num;
    }

    return *this;
}

template<class T, uintmax_t UDen>
constexpr rational<T, UDen>& rational<T, UDen>::operator -= (const rational<T, UDen>& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        *this = *this - right;
    }
    else
    {
        num -= right.num;
    }

    return *this;
}

template<class T>
[[nodiscard]] constexpr rational<T> operator * (const rational<T>& left, const rational<T>& right) noexcept
{
    const auto gcd_left = std::gcd(left.num, right.den);
    const auto gcd_right = std::gcd(right.num, left.den);

    return
    {
        (left.num / gcd_left) * (right.num / gcd_right),
        (left.den / gcd_right) * (right.den / gcd_left)
    };
}

template<class T, uintmax_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    is_safe_numeric_conversion_v<T, U>,
    rational<T, UDen>
>
operator * (const rational<T, UDen>& left, const U& right) noexcept
{
    if constexpr (UDen == dynamic_denominator)
    {
        const T right_num{ right };
        const auto gcd_right = std::gcd(right_num, left.den);

        return
        {
            left.num * (right_num / gcd_right),
            left.den / gcd_right
        };
    }
    else
    {
        return { left.num * right };
    }
}

template<class T, uintmax_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    is_safe_numeric_conversion_v<T, U>,
    rational<T, UDen>
>
operator * (const U& left, const rational<T, UDen>& right) noexcept
{
    return right * left;
}

template<class T, uintmax_t UDen, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    is_safe_numeric_conversion_v<T, U>,
    rational<T, UDen>
>
operator / (const rational<T, UDen>& left, const U& right) noexcept
{
    D_ASSERT_OR_ASSUME(right);

    if constexpr (UDen == dynamic_denominator)
    {
        const T right_num{ right };
        const auto gcd_right = std::gcd(right_num, left.num);

        return
        {
            left.num / gcd_right,
            left.den * (right_num / gcd_right)
        };
    }
    else
    {
        return { left.num / right };
    }
}


template<class T, class U>
[[nodiscard]] constexpr std::enable_if_t
<
    is_safe_numeric_conversion_v<T, U>,
    rational<T>
>
operator / (const U& left, const rational<T>& right) noexcept
{
    return left * inverse(right);
}

template<class T>
[[nodiscard]] constexpr rational<T> operator / (const rational<T>& left, const rational<T>& right) noexcept
{
    return left * inverse(right);
}

template<class T>
constexpr rational<T> parse_rational(span<const char> string) noexcept
{
    constexpr T max_v = numeric_max_v<T>;

    constexpr auto is_digit = [] (char c) noexcept
    {
        return c >= '0' && c <= '9';
    };

    auto r = rational<T>::zero();
    bool isdot = false;
    size_t index = 0;

    for (const auto c : string)
    {
        if (!is_digit(c))
        {
            if constexpr (std::is_signed_v<T>)
            {
                if (!index && c == '-')
                {
                    continue;
                }
            }

            if (c == '.')
            {
                isdot = true;
                continue;
            }

            D_ASSERT(!"invalid char");
            return r;
        }

        const char digit = (c - '0');

        if ((r.num > (max_v - digit) / 10) || (r.den > max_v / 10))
        {
            D_ASSERT(!"sint overflow");
            if (isdot)
            {
                break;
            }
            else
            {
                return r;
            }
        }

        r.num *= 10;
        r.num += digit;

        if (isdot)
        {
            r.den *= 10;
        }

        ++index;
    }

    if constexpr (std::is_signed_v<T>)
    {
        if (r.num && string.size() && string.front() == '-')
        {
            r.num = -r.num;
        }
    }

    return simplify(r);
}

template<class T, denomi_t UDen>
[[nodiscard]] constexpr rational<T, UDen> rational<T, UDen>::instance(span<const char> string) noexcept
{
    const auto r = parse_rational<T>(string);

    if constexpr (UDen == dynamic_denominator)
    {
        return r;
    }
    else
    {
        return { static_cast<T>(rational_to_float(r) * UDen) };
    }
}

namespace rational_literals
{
    template<char ... chars>
    [[nodiscard]] constexpr rational<ptrdiff_t> operator ""_r() noexcept
    {
        constexpr char string[] = { chars... };
        constexpr auto result = rational<ptrdiff_t>::instance(string);
        return result;
    }

    template<char ... chars>
    [[nodiscard]] constexpr rational<size_t> operator ""_ur() noexcept
    {
        constexpr char string[] = { chars... };
        constexpr auto result = rational<size_t>::instance(string);
        return result;
    }
}
