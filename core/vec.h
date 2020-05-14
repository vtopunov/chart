#pragma once

#include <algorithm>

#include <core/defs.h>

template<class T>
struct vec
{
    using value_type = T;

    T _0;
    T _1;

    constexpr T get(std::index_sequence<0u>) const noexcept
    {
        return _0;
    }

    constexpr T get(std::index_sequence<1u>) const noexcept
    {
        return _1;
    }

    template<size_t index>
    constexpr T get() const noexcept
    {
        return get(std::index_sequence<index>());
    }

    constexpr bool operator == (const vec&) const noexcept = default;

    constexpr bool operator != (const vec&) const noexcept = default;
};

template<class T>
constexpr vec<T> make_vec(const T& first, const T& second) noexcept
{
    return { first, second };
}

template<class T>
constexpr vec<T> fill_vec(const T& value) noexcept
{
    return { value, value };
}

template<class T>
constexpr vec<T> reverse(const vec<T>& v) noexcept
{
    return make_vec(v._1, v._0);
}

template<class T>
constexpr T sum(const vec<T>& v) noexcept
{
    return v._1 + v._0;
}

template<class T>
constexpr T difference(const vec<T>& v) noexcept
{
    return v._1 - v._0;
}

template<class T>
constexpr T mean(const vec<T>& v) noexcept
{
    return sum(v) / 2;
}

template<class T>
constexpr vec<T> operator - (const vec<T>& rigth) noexcept
{
    return { -rigth._0, -rigth._1 };
}

template<class T>
constexpr vec<T> operator - (const vec<T>& left, const vec<T>& right) noexcept
{
    return { left._0 - right._0, left._1 - right._1 };
}

template<class T>
constexpr vec<T> operator + (const vec<T>& left, const vec<T>& right) noexcept
{
    return { left._0 + right._0, left._1 + right._1 };
}

template<class T>
constexpr vec<T> operator * (const vec<T>& left, const T& right) noexcept
{
    return { left._0 * right, left._1 * right };
}

template<class T>
constexpr vec<T> operator * (const T left, const vec<T>& right) noexcept
{
    return right * left;
}

template<class T, class U>
constexpr std::enable_if_t<std::is_arithmetic_v<U>, vec<T>>  operator / (const vec<T>& left, const U& right) noexcept
{
    return { left._0 / right, left._1 / right };
}

template<class T>
constexpr vec<T> min(const vec<T>& a, const vec<T>& b) noexcept;

template<class T>
constexpr vec<T> max(const vec<T>& a, const vec<T>& b) noexcept;

template<class T>
constexpr T min(const vec<T>& v) noexcept
{
    using std::min;
    using ::min;
    return min(v._0, v._1);
}

template<class T>
constexpr T max(const vec<T>& v) noexcept
{
    using std::max;
    using ::max;
    return max(v._0, v._1);
}

template<class T>
constexpr vec<T> min(const vec<T>& a, const vec<T>& b) noexcept
{
    return
    {
        min(make_vec(a._0, b._0)),
        min(make_vec(a._1, b._1))
    };
}

template<class T>
constexpr vec<T> max(const vec<T>& a, const vec<T>& b) noexcept
{
    return
    {
        max(make_vec(a._0, b._0)),
        max(make_vec(a._1, b._1))
    };
}