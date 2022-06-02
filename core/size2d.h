#pragma once

#include <core/vec2.h>

template<class T>
struct size2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using vec2_type::_0;
    using vec2_type::_1;
    using reference = T&;
    using const_reference = const T&;

    [[nodiscard]]
    constexpr T width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr T height() const noexcept
    {
        return cref_height();
    }

    [[nodiscard]]
    constexpr reference ref_width() noexcept
    {
        return as_mutable(cref_width());
    }

    [[nodiscard]]
    constexpr reference ref_height() noexcept
    {
        return as_mutable(cref_height());
    }

    [[nodiscard]]
    constexpr const_reference ref_width() const noexcept
    {
        return cref_width();
    }

    [[nodiscard]]
    constexpr const_reference ref_height() const noexcept
    {
        return cref_height();
    }

    [[nodiscard]]
    constexpr const_reference cref_width() const noexcept
    {
        return _0;
    }

    [[nodiscard]]
    constexpr const_reference cref_height() const noexcept
    {
        return _1;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return is_positive(width()) && is_positive(height());
    }

    [[nodiscard]]
    constexpr bool operator == (const size2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const size2d&) const noexcept = default;
};


template<class T>
size2d(T, T)->size2d<T>;

template<class T>
size2d(const vec2<T>&)->size2d<T>;

template<class L, class R> [[nodiscard]]
constexpr decltype(auto) operator - (const size2d<L>& left, const size2d<R>& right) noexcept
{
    return size2d{ as_vec2(left) - as_vec2(right) };
}

template<class L, class R> [[nodiscard]]
constexpr decltype(auto) operator + (const size2d<L>& left, const size2d<R>& right) noexcept
{
    return size2d{ as_vec2(left) + as_vec2(right) };
}

template<class T> [[nodiscard]]
constexpr decltype(auto) operator * (const size2d<T>& left, const T& right) noexcept
{
    return size2d{ as_vec2(left) * right };
}

template<class T> [[nodiscard]]
constexpr decltype(auto)  operator * (const T& left, const size2d<T>& right) noexcept
{
    return right * left;
}

template<class T, class U> [[nodiscard]]
constexpr std::enable_if_t
<
    std::conjunction_v<std::negation<std::is_same<T, U>>, std::is_arithmetic<U>>,
    size2d<decl_mul_t<T, U>>
>
operator * (const size2d<T>& left, const U& right) noexcept
{
    return { as_vec2(left) * right };
}

template<class U, class T> [[nodiscard]]
constexpr auto operator * (const U& left, const size2d<T>& right) noexcept -> decltype(right* left)
{
    return right * left;
}