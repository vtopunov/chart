#pragma once

#include <core/defs.h>
#include <core/span.h>

template<class T>
class rational
{
public:
    constexpr rational() noexcept = default;

    constexpr rational(T integer) noexcept
        : num_{ std::move(integer) }
    {}

    constexpr rational(T num_part, T den_part) noexcept
        : num_{ std::move(num_part) }
        , den_{ std::move(den_part) }
    {
        D_ASSERT(den_);
    }

    constexpr T num() const noexcept
    {
        return num_;
    }

    constexpr T den() const noexcept
    {
        return den_;
    }

    constexpr T to_integer() const noexcept
    {
        return num_ / den_;
    }

    static constexpr rational<T> from_string(span<const char> string) noexcept;

    constexpr bool operator == (const rational&) const noexcept = default;
    constexpr bool operator != (const rational&) const noexcept = default;

private:
    T num_{};
    T den_{ 1u };
};

template<class T>
constexpr std::enable_if_t<std::is_signed_v<T>, rational<T>> operator - (const rational<T>& left) noexcept
{
    return { -left.num(), left.den() };
}

template<class T>
constexpr rational<T> operator + (const rational<T>& left, const rational<T>& right) noexcept
{
    const auto gcd = std::gcd(left.den(), right.den());
    const auto mul_left = right.den() / gcd;
    const auto mul_right = left.den() / gcd;

    return
    {
        left.num() * narrow_cast<T>( mul_left ) + right.num() * narrow_cast<T>( mul_right ),
        mul_left * left.den()
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U>,
    rational<std::common_type_t<T, U>>
>
operator + (const rational<T>& left, const U& right) noexcept
{
    return
    {
        left.num() + left.den() * narrow_cast<std::common_type_t<T, U>>( right ),
        left.den()
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U>,
    rational<std::common_type_t<T, U>>
>
operator + (const U& left, const rational<T>& right) noexcept
{
    return right + left;
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U> && std::is_signed_v<U> && std::is_signed_v<T>,
    rational<std::common_type_t<T, U>>
>
operator - (const rational<T>& left, const U& right) noexcept
{
    return left + ( -right );
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U> && std::is_signed_v<U> && std::is_signed_v<T>,
    rational<std::common_type_t<T, U>>
>
operator - (const U& left, const rational<T>& right) noexcept
{
    return left + ( -right );
}

template<class T>
constexpr std::enable_if_t
<
    std::is_signed_v<T>,
    rational<T>
>
operator - (const rational<T>& left, const rational<T>& right) noexcept
{
    return left + ( -right );
}

template<class T>
constexpr rational<T> operator * (const rational<T>& left, const rational<T>& right) noexcept
{
    const auto gcd_left = std::gcd(left.num(), right.den());
    const auto gcd_right = std::gcd(right.num(), left.den());

    return
    {
        ( left.num() / narrow_cast<T>( gcd_left ) ) * ( right.num() / narrow_cast<T>( gcd_right ) ),
        ( left.den() / gcd_right ) * ( right.den() / gcd_left )
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U>,
    rational<std::common_type_t<T, U>>
>
operator * (const rational<T>& left, const U& right) noexcept
{
    return
    {
        left.num() * right,
        left.den()
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U>,
    rational<std::common_type_t<T, U>>
>
operator * (const U& left, const rational<T>& right) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U> && std::is_signed_v<T> && std::is_signed_v<U>,
    rational<std::common_type_t<T, U>>
>
operator / (const rational<T>& left, const U& right) noexcept
{
    const auto is_sign = right < 0;
    return
    {
        ( is_sign ) ? -left.num() : left.num(),
        left.den() * ( ( is_sign ) ? -right : right )
    };
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U> && std::is_unsigned_v<T> && std::is_unsigned_v<U>, 
    rational<std::common_type_t<T, U>> 
> 
operator / (const rational<T>& left, const U& right) noexcept
{
    return
    {
        left.num(),
        left.den() * right
    };
}


template<class T>
constexpr std::enable_if_t
< 
    std::is_signed_v<T>, 
    rational<T> 
> 
inverse(const rational<T>& value) noexcept
{
    const auto with_negative = value.num() < 0;

    return
    {
        with_negative ? -value.den() : value.den(),
        with_negative ? -value.num() : value.num()
    };
}

template<class T>
constexpr std::enable_if_t
<
    std::is_unsigned_v<T>, 
    rational<T>
> 
inverse(const rational<T>& value) noexcept
{
    return { value.den(), value.num() };
}


template<class T>
constexpr rational<T> operator / (const rational<T>& left, const rational<T>& right) noexcept
{
    return left * inverse(right);
}

template<class T, class U>
constexpr std::enable_if_t
<
    std::is_integral_v<U>, 
    rational<std::common_type_t<T, U>> 
> 
operator / (const U& left, const rational<T>& right) noexcept
{
    return left * inverse(right);
}

template<class T>
constexpr rational<T> simplify(const rational<T>& value) noexcept
{
    const auto gcd = std::gcd(value.num(), value.den());
    return
    {
        value.num() / narrow_cast<T>( gcd ),
        value.den() / gcd
    };
}

template<class T>
constexpr T to_integer(const rational<T>& value) noexcept
{
    return value.to_integer();
}

template<class T>
constexpr bool operator < (const rational<T>& left, const rational<T>& right) noexcept
{
    const auto gcd = std::gcd(left.den(), right.den());
    const auto mul_left = right.den() / gcd;
    const auto mul_right = left.den() / gcd;

    return left.num() * narrow_cast<T>( mul_left ) < right.num() * narrow_cast<T>( mul_right );
}

template<class T>
constexpr bool operator <= (const rational<T>& left, const rational<T>& right) noexcept
{
    return left == right || left < right;
}

template<class T>
constexpr bool operator > (const rational<T>& left, const rational<T>& right) noexcept
{
    return right < left;
}

template<class T>
constexpr bool operator >= (const rational<T>& left, const rational<T>& right) noexcept
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

    rational<T> r;
    bool isdot = false;
    size_t index = 0;

    for ( const auto c : string )
    {
        if ( !is_digit(c) )
        {
            if constexpr ( std::is_signed_v<T> )
            {
                if ( !index && c == '-' )
                {
                    continue;
                }
            }

            if ( c == '.' )
            {
                isdot = true;
                continue;
            }

            D_ASSERT(!"invalid char");
            return r;
        }

        const char digit = ( c - '0' );

        if ( ( r.num() > ( max - digit ) / 10 ) || ( r.den() > max / 10 ) )
        {
            D_ASSERT(!"integer overflow");
            if ( isdot )
            {
                break;
            }
            else
            {
                return r;
            }
        }

        r.num_ *= 10;
        r.num_ += digit;

        if ( isdot )
        {
            r.den_ *= 10;
        }

        ++index;
    }

    if constexpr ( std::is_signed_v<T> )
    {
        if ( r.num_ && string.size() && string.front() == '-' )
        {
            r.num_ = -r.num_;
        }
    }

    return simplify(r);
}

template<char ... String>
constexpr rational<ptrdiff_t> operator"" _r() noexcept
{
    constexpr char string[] = { String... };
    return rational<ptrdiff_t>::from_string(string);
}

template<char ... String>
constexpr rational<size_t> operator"" _ur() noexcept
{
    constexpr char string[] = { String... };
    return rational<size_t>::from_string(string);
}
