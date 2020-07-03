#pragma once

#include <cstdint>
#include <compare>
#include <numeric>

#include <core/span.h>

template<class T>
using upgrade_int_t = std::conditional_t
<
    is_narrowing_v<T, ptrdiff_t>, ptrdiff_t,
    std::conditional_t<is_narrowing_v<T, intmax_t>, intmax_t, T>
>;

template<class T>
struct rational
{
    using int_type = T;
    static_assert(std::is_integral_v<int_type>);

    int_type num;
    int_type den;

    explicit constexpr operator bool() const noexcept
    {
        return !!num;
    }

    constexpr bool operator == (const rational&) const noexcept = default;

    constexpr bool operator != (const rational&) const noexcept = default;

    static constexpr rational zero() noexcept
    {
        return from_int(0);
    }

    static constexpr rational from_int(int_type num) noexcept
    {
        constexpr int_type one = 1;
        return { num, one };
    }

    static constexpr rational from_string(span<const char> string) noexcept;

    using upgrade_int_type = upgrade_int_t<int_type>;

    template<
        bool enable_bool = true,
        class = std::enable_if_t<(!std::is_same_v<int_type, upgrade_int_type>&& enable_bool)>
    >
        constexpr operator rational<upgrade_int_type>() const noexcept
    {
        return
        {
            num,
            den
        };
    }
};

template<class T>
rational(T, T)->rational<T>;

template <class T>
struct is_rational : public std::false_type
{};

template <class T>
struct is_rational<rational<T>> : public std::true_type
{};

template<class T>
inline constexpr bool is_rational_v = is_rational<T>::value;

template<class T>
constexpr rational<T> simplify(T num, T den) noexcept
{
    D_ASSERT(den);

    const auto gcd = std::gcd(num, den);

    return
    {
        num / gcd,
        den / gcd
    };
}

template<class T>
constexpr rational<T> simplify(const rational<T> value) noexcept
{
    return simplify(value.num, value.den);
}

template<class T>
constexpr rational<T> inverse(const rational<T> value) noexcept
{
    D_ASSERT(value.num);

    return
    {
        value.den,
        value.num
    };
}

template<class Target, class Source>
constexpr Target rational_cast(const Source src) noexcept
{
    if constexpr (is_rational_v<Source>)
    {
        D_ASSERT(src.den);

        if constexpr (is_rational_v<Target>)
        {
            using int_type = typename Target::int_type;
            return
            {
                narrow_cast<int_type>(src.num),
                narrow_cast<int_type>(src.den)
            };
        }
        else
        {
            if constexpr (std::is_integral_v<Target>)
            {
                return narrow_cast<Target>(src.num / src.den);
            }
            else
            {
                static_assert(std::is_floating_point_v<Target>);
                return static_cast<Target>(src.num) / static_cast<Target>(src.den);
            }
        }
    }
    else
    {
        static_assert(is_rational_v<Target> && std::is_integral_v<Source>);
        using int_type = typename Target::int_type;
        return Target::from_int(narrow_cast<int_type>(src));
    }
}

template<class T>
constexpr std::enable_if_t<std::is_signed_v<T>, rational<T> > operator - (const rational<T> left) noexcept
{
    return { -left.num, left.den };
}

template<class T>
constexpr rational<T> operator + (const rational<T> left, const rational<T> right) noexcept
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

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>,
    rational<T>
>
operator + (const rational<T> left, const U right) noexcept
{
    return
    {
        left.num + left.den * T{ right },
        left.den
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>,
    rational<T>
>
operator + (const U left, const rational<T> right) noexcept
{
    return right + left;
}

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>
    && std::is_signed_v<T>
    && std::is_signed_v<U>,
    rational<T>
>
operator - (const rational<T> left, const U right) noexcept
{
    return
    {
        left.num - left.den * T{ right },
        left.den
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>
    && std::is_signed_v<T>
    && std::is_signed_v<U>,
    rational<T>
>
operator - (const U left, const rational<T> right) noexcept
{
    return
    {
        left.den * T{ left } - right.num,
        left.den
    };
}

template<class T>
constexpr std::enable_if_t
<
    std::is_signed_v<T>,
    rational<T>
> operator - (const rational<T> left, const rational<T> right) noexcept
{
    return left + (-right);
}

template<class T>
constexpr rational<T> operator * (const rational<T> left, const rational<T> right) noexcept
{
    const auto gcd_left = std::gcd(left.num, right.den);
    const auto gcd_right = std::gcd(right.num, left.den);

    return
    {
        (left.num / gcd_left) * (right.num / gcd_right),
        (left.den / gcd_right) * (right.den / gcd_left)
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>,
    rational<T>
>
operator * (const rational<T> left, const U right) noexcept
{
    const T right_num{ right };
    const auto gcd_right = std::gcd(right_num, left.den);

    return
    {
        left.num * (right_num / gcd_right),
        left.den / gcd_right
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>,
    rational<T>
>
operator * (const U left, const rational<T> right) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>,
    rational<T>
>
operator / (const rational<T> left, const U right) noexcept
{
    D_ASSERT(right);

    const T right_num{ right };
    const auto gcd_right = std::gcd(right_num, left.num);

    return
    {
        left.num / gcd_right,
        left.den * (right_num / gcd_right)
    };
}


template<class T, class U>
constexpr std::enable_if_t
<
    is_safe_integral_conversion_v<T, U>,
    rational<T>
>
operator / (const U left, const rational<T> right) noexcept
{
    return left * inverse(right);
}

template<class T>
constexpr rational<T> operator / (const rational<T> left, const rational<T> right) noexcept
{
    return left * inverse(right);
}

template<class T>
constexpr bool operator < (const rational<T> left, const rational<T> right) noexcept
{
    const auto gcd = std::gcd(left.den, right.den);
    const auto mul_left = right.den / gcd;
    const auto mul_right = left.den / gcd;

    return left.num * mul_left < right.num* mul_right;
}

template<class T>
constexpr bool operator <= (const rational<T> left, const rational<T> right) noexcept
{
    return left == right || left < right;
}

template<class T>
constexpr bool operator > (const rational<T> left, const rational<T> right) noexcept
{
    return right < left;
}

template<class T>
constexpr bool operator >= (const rational<T> left, const rational<T> right) noexcept
{
    return left == right || left > right;
}

template<class T>
constexpr rational<T> rational<T>::from_string(span<const char> string) noexcept
{
    constexpr T max = std::numeric_limits<T>::max();

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

        if ((r.num > (max - digit) / 10) || (r.den > max / 10))
        {
            D_ASSERT(!"integer overflow");
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

namespace rational_literals
{
    template<char ... chars>
    constexpr rational<ptrdiff_t> operator"" _r() noexcept
    {
        constexpr char string[] = { chars... };
        return rational<ptrdiff_t>::from_string(string);
    }

    template<char ... chars>
    constexpr rational<size_t> operator"" _ur() noexcept
    {
        constexpr char string[] = { chars... };
        return rational<size_t>::from_string(string);
    }
}
