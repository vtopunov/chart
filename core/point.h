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
        return position.get<underlying_cast<size_t>(axis)>();
    }

    constexpr bool operator == (const point&) const noexcept = default;

    constexpr bool operator != (const point&) const noexcept = default;
};

template<class T>
point(vec<T>)->point<T>;

template<class T>
point(T, T)->point<T>;

template<class T>
constexpr point<T> operator - (const point<T>& v) noexcept
{
    return { -v.position };
}

template<class T>
constexpr point<T> operator - (const point<T>& left, const point<T>& right) noexcept
{
    return { left.position - right.position };
}

template<class T>
constexpr point<T> operator + (const point<T>& left, const point<T>& right) noexcept
{
    return { left.position + right.position };
}

template<class T>
constexpr point<T> operator * (const point<T>& left, const T& right) noexcept
{
    return { left.position * right };
}

template<class T>
constexpr point<T> operator * (const T& left, const point<T>& right) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t<std::is_arithmetic_v<U>, point<T>> operator / (const point<T>& left, const U& right) noexcept
{
    return { left.position / right };
}

template<class T>
constexpr point<T> min(const point<T>& a, const point<T>& b) noexcept
{
    return { min(a.position, b.position) };
}

template<class T>
constexpr point<T> max(const point<T>& a, const point<T>& b) noexcept
{
    return { max(a.position, b.position) };
}