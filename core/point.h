#pragma once

#include <core/vec.h>
#include <core/axis_type.h>
#include <core/underlying_cast.h>

template<class T>
struct point
{
    vec<T> position{};

    constexpr point() noexcept = default;

    constexpr point(T x, T y) noexcept
        : position{ x, y }
    {}

    explicit constexpr point(vec<T> v) noexcept
        : position{ v }
    {}

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

    static constexpr point fill( T value ) noexcept
    {
        return point{ vec<T>::fill( value ) };
    }
};

template<class T>
constexpr point<T> operator - (point<T> v) noexcept
{
    return point<T>(-v.position);
}

template<class T>
constexpr point<T> operator - (point<T> left, point<T> right) noexcept
{
    return point<T>(left.position - right.position);
}

template<class T>
constexpr point<T> operator + ( point<T> left, point<T> right ) noexcept
{
    return point<T>( left.position + right.position );
}

template<class T>
constexpr point<T> operator * (point<T> left, T right) noexcept
{
    return point<T>(left.position * right);
}

template<class T>
constexpr point<T> operator * (T left, point<T> right) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t<std::is_arithmetic_v<U>, point<T>> operator / (point<T> left, U right) noexcept
{
    return point<T>(left.position / right);
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
    return point<T>(min(a.position, b.position));
}

template<class T>
constexpr point<T> max(point<T> a, point<T> b) noexcept
{
    return point<T>(max(a.position, b.position));
}

using point_t = point<real_t>;