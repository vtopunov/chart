#pragma once

#include <algorithm>
#include <core/util.h>

template<class T>
struct vec;

template<class T>
constexpr vec<T> min( vec<T> a, vec<T> b ) noexcept;

template<class T>
constexpr vec<T> max( vec<T> a, vec<T> b ) noexcept;

namespace vec_private_detail
{
    template<class T>
    constexpr T private_max( T a, T b ) noexcept
    {
        using std::max;
        using ::max;
        return max( a, b );
    }

    template<class T>
    constexpr T private_min( T a, T b ) noexcept
    {
        using std::min;
        using ::min;
        return min( a, b );
    }
}

template<class T>
struct vec
{
    T _0{};
    T _1{};

    constexpr vec() noexcept = default;

    constexpr vec( T elem0, T elem1 ) noexcept
        : _0{ elem0 }
        , _1{ elem1 }
    {}

    constexpr T get( std::index_sequence<0_z> ) const noexcept
    {
        return _0;
    }

    constexpr T get( std::index_sequence<1_z> ) const noexcept
    {
        return _1;
    }

    template<size_t index>
    constexpr T get() const noexcept
    {
        return get( std::index_sequence<index>() );
    }

    constexpr T min() const noexcept
    {
        return vec_private_detail::private_min( _0, _1 );
    }

    constexpr T max() const noexcept
    {
        return vec_private_detail::private_max( _0, _1 );
    }

    constexpr T sum() const noexcept
    {
        return _0 + _1;
    }

    constexpr T difference() const noexcept
    {
        return _1 - _0;
    }

    constexpr T mean() const noexcept
    {
        return sum() / 2;
    }

    constexpr vec with_reverse() const noexcept
    {
        return { _1, _0 };
    }

    static constexpr vec<T> fill( T value ) noexcept
    {
        return { value, value };
    }
};

template<class T>
constexpr vec<T> operator - ( vec<T> rigth ) noexcept
{
    return { -rigth._0, -rigth._1 };
}

template<class T>
constexpr vec<T> operator - ( vec<T> left, vec<T> right ) noexcept
{
    return { left._0 - right._0, left._1 - right._1 };
}

template<class T>
constexpr vec<T> operator + ( vec<T> left, vec<T> right ) noexcept
{
    return { left._0 + right._0, left._1 + right._1 };
}

template<class T>
constexpr vec<T> operator * ( vec<T> left, T right ) noexcept
{
    return { left._0 * right, left._1 * right };
}

template<class T>
constexpr vec<T> operator * ( T left, vec<T> right ) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t<std::is_arithmetic_v<U>, vec<T>>  operator / ( vec<T> left, U right ) noexcept
{
    return { left._0 / right, left._1 / right };
}


template<class T>
constexpr bool operator == ( vec<T> left, vec<T> right ) noexcept
{
    return left._0 == right._0 && left._1 == right._1;
}

template<class T>
constexpr bool operator != ( vec<T> left, vec<T> right ) noexcept
{
    return !( left == right );
}

template<class T>
constexpr bool operator < ( vec<T> left, vec<T> right ) noexcept
{
    return left._0 < right._0 && left._1 < right._1;
}

template<class T>
constexpr bool operator <= ( vec<T> left, vec<T> right ) noexcept
{
    return left < right || left == right;
}

template<class T>
constexpr bool operator > ( vec<T> left, vec<T> right ) noexcept
{
    return left._0 > right._0 && left._1 > right._1;
}

template<class T>
constexpr bool operator >= ( vec<T> left, vec<T> right ) noexcept
{
    return left > right || left == right;
}

template<class T>
constexpr vec<T> min( vec<T> a, vec<T> b ) noexcept
{
    return
    {
        vec<T>{ a._0, b._0 }.min(),
        vec<T>{ a._1, b._1 }.min()
    };
}

template<class T>
constexpr vec<T> max( vec<T> a, vec<T> b ) noexcept
{
    return
    {
        vec<T>{ a._0, b._0 }.max(),
        vec<T>{ a._1, b._1 }.max()
    };
}

using vec_t = vec<real_t>;