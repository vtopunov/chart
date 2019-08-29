#pragma once

#include "util.h"
#include "span.h"

template<class T>
class rational
{
public:
    using Unsigned = std::make_unsigned_t<T>;

    constexpr rational() noexcept = default;

    constexpr rational( T integer ) noexcept
        : num_{ integer }
    {}

    constexpr rational( T num_part, Unsigned den_part ) noexcept
        : num_{ num_part }
        , den_{ den_part }
    {
        assert( den_ );
    }

    constexpr T num() const noexcept
    {
        return num_;
    }

    constexpr Unsigned den() const noexcept
    {
        return den_;
    }

    constexpr T to_integer() const noexcept
    {
        return num_ / den_;
    }

    static constexpr rational<T> from_string( span<const char> string ) noexcept;

private:
    T num_{};
    Unsigned den_{ 1u };
};

template<class T>
constexpr std::enable_if_t< std::is_signed_v<T>, rational<T> > operator - ( rational<T> left ) noexcept
{
    return { -left.num(), left.den() };
}

template<class T>
constexpr rational<T> operator + ( rational<T> left, rational<T> right ) noexcept
{
    const auto gcd = std::gcd( left.den(), right.den() );
    const auto mul_left = right.den() / gcd;
    const auto mul_right = left.den() / gcd;

    return
    {
        left.num() * narrow_cast<T>( mul_left ) + right.num() * narrow_cast<T>( mul_right ),
        mul_left * left.den()
    };
}

template<class T, class U>
constexpr std::enable_if_t < std::is_integral_v<U>, rational< std::common_type_t<T, U> > > operator + ( rational<T> left, U right ) noexcept
{
    return
    {
        left.num() + left.den() * narrow_cast<std::common_type_t<T, U>>( right ),
        left.den()
    };
}

template<class T, class U>
constexpr std::enable_if_t < std::is_integral_v<U>, rational< std::common_type_t<T, U> > > operator + ( U left, rational<T> right ) noexcept
{
    return right + left;
}

template<class T, class U>
constexpr std::enable_if_t < std::is_integral_v<U> && std::is_signed_v<U> && std::is_signed_v<T>, rational< std::common_type_t<T, U> > > operator - ( rational<T> left, U right ) noexcept
{
    return left + ( -right );
}

template<class T, class U>
constexpr std::enable_if_t < std::is_integral_v<U> && std::is_signed_v<U> && std::is_signed_v<T>, rational< std::common_type_t<T, U> > > operator - ( U left, rational<T> right ) noexcept
{
    return left + ( -right );
}

template<class T>
constexpr std::enable_if_t < std::is_signed_v<T>, rational<T> > operator - ( rational<T> left, rational<T> right ) noexcept
{
    return left + ( -right );
}

template<class T>
constexpr rational<T> operator * ( rational<T> left, rational<T> right ) noexcept
{
    const auto gcd_left = std::gcd( left.num(), right.den() );
    const auto gcd_right = std::gcd( right.num(), left.den() );

    return
    {
        ( left.num() / narrow_cast<T>( gcd_left ) ) * ( right.num() / narrow_cast<T>( gcd_right ) ),
        ( left.den() / gcd_right ) * ( right.den() / gcd_left )
    };
}

template<class T, class U>
constexpr std::enable_if_t< std::is_integral_v<U>, rational<std::common_type_t<T, U>> > operator * ( rational<T> left, U right ) noexcept
{
    return
    {
        left.num() * right,
        left.den()
    };
}

template<class T, class U>
constexpr std::enable_if_t < std::is_integral_v<U>, rational<std::common_type_t<T, U>> > operator * ( U left, rational<T> right ) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t<std::is_integral_v<U> && std::is_signed_v<T> && std::is_signed_v<U>, rational<std::common_type_t<T, U>> > operator / ( rational<T> left, U right ) noexcept
{
    const auto is_sign = right < 0;
    return
    {
        (is_sign) ? -left.num() : left.num(),
        left.den() * to_unsingned( (is_sign) ? -right : right )
    };
}

template<class T, class U>
constexpr std::enable_if_t<std::is_integral_v<U> && std::is_unsigned_v<T> && std::is_unsigned_v<U>, rational<std::common_type_t<T, U>> > operator / ( rational<T> left, U right ) noexcept
{
    return
    {
        left.num(),
        left.den() * right
    };
}


template<class T>
constexpr std::enable_if_t< std::is_signed_v<T>, rational<T> > inverse( rational<T> value ) noexcept
{
    const auto signed_den = to_singned( value.den() );
    const auto with_negative = value.num() < 0;

    return
    {
        with_negative ? -signed_den : signed_den,
        to_unsingned( with_negative ? -value.num() : value.num() )
    };
}

template<class T>
constexpr std::enable_if_t<std::is_unsigned_v<T>, rational<T>> inverse( rational<T> value ) noexcept
{
    return { value.den(), value.num() };
}


template<class T>
constexpr rational<T> operator / ( rational<T> left, rational<T> right ) noexcept
{
    return left * inverse( right );
}

template<class T, class U>
constexpr std::enable_if_t<std::is_integral_v<U>, rational<std::common_type_t<T, U>> > operator / ( U left, rational<T> right ) noexcept
{
    return left * inverse( right );
}

template<class T>
constexpr rational<T> simplify( rational<T> value ) noexcept
{
    const auto gcd = std::gcd( value.num(), value.den() );
    return
    {
        value.num() / narrow_cast<T>( gcd ),
        value.den() / gcd
    };
}

template<class T>
constexpr T to_integer( rational<T> value ) noexcept
{
    return value.to_integer();
}

template<class T>
constexpr bool operator == ( rational<T> left, rational<T> right ) noexcept
{
    return left.num() == right.num() && left.den() == right.den();
}

template<class T>
constexpr bool operator != ( rational<T> left, rational<T> right ) noexcept
{
    return !( left == right );
}

template<class T>
constexpr bool operator < ( rational<T> left, rational<T> right ) noexcept
{
    const auto gcd = std::gcd( left.den(), right.den() );
    const auto mul_left = right.den() / gcd;
    const auto mul_right = left.den() / gcd;

    return left.num() * narrow_cast<T>( mul_left ) < right.num() * narrow_cast<T>( mul_right );
}

template<class T>
constexpr bool operator <= ( rational<T> left, rational<T> right ) noexcept
{
    return left == right || left < right;
}

template<class T>
constexpr bool operator > ( rational<T> left, rational<T> right ) noexcept
{
    return right < left;
}

template<class T>
constexpr bool operator >= ( rational<T> left, rational<T> right ) noexcept
{
    return left == right || left > right;
}

using rational_t = rational<int_t>;
using urational_t = rational<size_t>;

template<class T>
constexpr rational<T> rational<T>::from_string( span<const char> string ) noexcept
{
    constexpr T max = std::numeric_limits<T>::max();

    rational<T> r;
    bool isdot = false;
    size_t index = 0;

    for ( const auto c : string )
    {
        if ( !is_digit( c ) )
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

            assert( !"invalid char" );
            return r;
        }

        const char digit = ( c - '0' );

        if ( ( r.num() > ( max - digit ) / 10 ) || ( r.den() > max / 10 ) )
        {
            assert( !"integer overflow" );
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

    return simplify( r );
}

template<char ... String>
constexpr rational_t operator"" _r() noexcept
{
    constexpr char string[] = { String... };
    return rational_t::from_string( string );
}

template<char ... String>
constexpr urational_t operator"" _ur() noexcept
{
    constexpr char string[] = { String... };
    return urational_t::from_string( string );
}
