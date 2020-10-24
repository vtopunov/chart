#pragma once

#include <type_traits>
#include <algorithm>
#include <span>

#include <core/underlying_cast.h>

#undef min
#undef max

template<class T>
struct vec2
{
    using value_type = T;

    T _0;
    T _1;

    [[nodiscard]]
    constexpr T x() const noexcept
    {
        return _0;
    }

    [[nodiscard]]
    constexpr T y() const noexcept
    {
        return _1;
    }

    using const_span_type = std::span<std::add_const_t<value_type>, 2u>;

    [[nodiscard]]
    constexpr operator const_span_type() const noexcept
    {
        return const_span_type{ std::addressof(_0), const_span_type::extent };
    }

    [[nodiscard]]
    constexpr bool operator == (const vec2&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const vec2&) const noexcept = default;
};

template<class T>
vec2(T, T)->vec2<T>;

template<class T> [[nodiscard]]
constexpr vec2<T> fill_vec(const T& value) noexcept
{
    return
    {
        value,
        value
    };
}

template<class T> [[nodiscard]]
constexpr vec2<T> shift_push_back(const vec2<T>& vec2, const T& value) noexcept
{
    return
    {
        vec2._1,
        value
    };
}

template<class T> [[nodiscard]]
constexpr vec2<T> shift_push_front(const vec2<T>& vec2, const T& value) noexcept
{
    return
    {
        value,
        vec2._0
    };
}


template<class T> [[nodiscard]]
constexpr vec2<T> reverse(const vec2<T>& v) noexcept
{
    return
    {
        v._1,
        v._0
    };
}

template<class T> [[nodiscard]]
constexpr decltype( auto ) sum(const vec2<T>& v) noexcept
{
    return v._1 + v._0;
}

template<class T> [[nodiscard]]
constexpr decltype( auto )  difference(const vec2<T>& v) noexcept
{
    return v._1 - v._0;
}

template<class T> [[nodiscard]]
constexpr decltype( auto ) mean(const vec2<T>& v) noexcept
{
    return sum(v) / 2;
}

template<class T> [[nodiscard]]
constexpr decltype( auto ) operator - (const vec2<T>& right) noexcept
{
    return vec2
    {
        -right._0,
        -right._1
    };
}

template<class T> [[nodiscard]]
constexpr decltype( auto ) operator - (const vec2<T>& left, const vec2<T>& right) noexcept
{
    return vec2
    {
        left._0 - right._0,
        left._1 - right._1
    };
}

template<class T> [[nodiscard]]
constexpr decltype( auto ) operator + (const vec2<T>& left, const vec2<T>& right) noexcept
{
    return vec2
    {
        left._0 + right._0,
        left._1 + right._1
    };
}

template<class T> [[nodiscard]]
constexpr decltype( auto ) operator * (const vec2<T>& left, const T& right) noexcept
{
    return vec2
    {
        left._0 * right,
        left._1 * right
    };
}

template<class T> [[nodiscard]]
constexpr decltype( auto )  operator * (const T& left, const vec2<T>& right) noexcept
{
    return vec2
    {
        right * left
    };
}

template<class T, class U> [[nodiscard]]
constexpr std::enable_if_t<std::is_arithmetic_v<U>, vec2<T>>  operator / (const vec2<T>& left, const U& right) noexcept
{
    return
    {
        left._0 / right,
        left._1 / right
    };
}

template<class T> [[nodiscard]]
constexpr vec2<T> min(const vec2<T>& a, const vec2<T>& b) noexcept;

template<class T> [[nodiscard]]
constexpr vec2<T> max(const vec2<T>& a, const vec2<T>& b) noexcept;

template<class T> [[nodiscard]]
constexpr T min(const vec2<T>& v) noexcept
{
    using std::min;
    using ::min;
    return min(v._0, v._1);
}

template<class T> [[nodiscard]]
constexpr T max(const vec2<T>& v) noexcept
{
    using std::max;
    using ::max;
    return max(v._0, v._1);
}

template<class T> [[nodiscard]]
constexpr vec2<T> min(const vec2<T>& a, const vec2<T>& b) noexcept
{
    return
    {
        min(vec2{ a._0, b._0 }),
        min(vec2{ a._1, b._1 })
    };
}

template<class T> [[nodiscard]]
constexpr vec2<T> max(const vec2<T>& a, const vec2<T>& b) noexcept
{
    return
    {
        max(vec2{ a._0, b._0 }),
        max(vec2{ a._1, b._1 })
    };
}