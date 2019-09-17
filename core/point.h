#pragma once

#include <core/vec.h>
#include <core/axis_type.h>
#include <core/underlying_cast.h>

template<class T>
struct point
{
    vec<T> position;

    constexpr T x() const noexcept
    {
        return position._0;
    }

    constexpr T y() const noexcept
    {
        return position._1;
    }

    template<axis_type axis>
    constexpr T get() const noexcept
    {
        return position.get<underlying_cast<size_t>( axis )>();
    }
};

template<class T>
constexpr point<T> make_point( T x, T y ) noexcept
{
    return { { x, y } };
}

template<class T>
constexpr point<T> operator - (point<T> v) noexcept
{
    return { -v.position };
}

template<class T>
constexpr point<T> operator - (point<T> left, point<T> right) noexcept
{
    return { left.position - right.position };
}

template<class T>
constexpr point<T> operator + ( point<T> left, point<T> right ) noexcept
{
    return { left.position + right.position };
}

template<class T>
constexpr point<T> operator * (point<T> left, T right) noexcept
{
    return { left.position * right };
}

template<class T>
constexpr point<T> operator * (T left, point<T> right) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t<std::is_arithmetic_v<U>, point<T>> operator / (point<T> left, U right) noexcept
{
    return { left.position / right };
}

template<class T>
constexpr bool operator == (point<T> left, point<T> right) noexcept
{
    return left.position == right.position;
}

template<class T>
constexpr bool operator != (point<T> left, point<T> right) noexcept
{
    return !( left == right );
}

template<class T>
constexpr bool operator < (point<T> left, point<T> right) noexcept
{
    return left.position < right.position;
}

template<class T>
constexpr bool operator <= (point<T> left, point<T> right) noexcept
{
    return left.position <= right.position;
}

template<class T>
constexpr bool operator > (point<T> left, point<T> right) noexcept
{
    return left.position > right.position;
}

template<class T>
constexpr bool operator >= (point<T> left, point<T> right) noexcept
{
    return left.position >= right.position;
}

template<class T>
constexpr point<T> min(point<T> a, point<T> b) noexcept
{
    return { min( a.position, b.position ) };
}

template<class T>
constexpr point<T> max(point<T> a, point<T> b) noexcept
{
    return { max( a.position, b.position ) };
}

using point_t = point<real_t>;