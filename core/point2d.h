#pragma once

#include <core/vec2.h>


template<class T>
struct point2d : vec2<T>
{
    using vec2_type = vec2<T>;
    using vec2_type::_0;
    using vec2_type::_1;
    using reference = T&;
    using const_reference = const T&;

    [[nodiscard]]
    constexpr T x() const noexcept
    {
        return cref_x();
    }

    [[nodiscard]]
    constexpr T y() const noexcept
    {
        return cref_y();
    }

    [[nodiscard]]
    constexpr reference ref_x() noexcept
    {
        return as_mutable(cref_x());
    }

    [[nodiscard]]
    constexpr reference ref_y() noexcept
    {
        return as_mutable(cref_y());
    }

    [[nodiscard]]
    constexpr const_reference ref_x() const noexcept
    {
        return cref_x();
    }

    [[nodiscard]]
    constexpr const_reference ref_y() const noexcept
    {
        return cref_y();
    }

    [[nodiscard]]
    constexpr const_reference cref_x() const noexcept
    {
        return _0;
    }

    [[nodiscard]]
    constexpr const_reference cref_y() const noexcept
    {
        return _1;
    }

    [[nodiscard]]
    constexpr bool operator == (const point2d&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const point2d&) const noexcept = default;
};

template<class T>
point2d(T, T)->point2d<T>;

template<class T>
point2d(const vec2<T>&)->point2d<T>;

template<class T>
[[nodiscard]] constexpr auto operator - (const point2d<T>& right) noexcept -> decltype(point2d{ -as_vec2(right) })
{
    return point2d{ -as_vec2(right) };
}

template<class L, class R>
[[nodiscard]] constexpr decltype(auto) operator - (const point2d<L>& left, const point2d<R>& right) noexcept
{
    return point2d{ as_vec2(left) - as_vec2(right) };
}

template<class L, class R>
[[nodiscard]] constexpr decltype(auto) operator + (const point2d<L>& left, const point2d<R>& right) noexcept
{
    return point2d{ as_vec2(left) + as_vec2(right) };
}

template<class T>
[[nodiscard]] constexpr decltype(auto) operator * (const point2d<T>& left, const T& right) noexcept
{
    return point2d{ as_vec2(left) * right };
}

template<class T>
[[nodiscard]] constexpr decltype(auto)  operator * (const T& left, const point2d<T>& right) noexcept
{
    return right * left;
}

template<class T, class U>
[[nodiscard]] constexpr std::enable_if_t<
    std::conjunction_v<std::negation<std::is_same<T, U>>, std::is_arithmetic<U>>,
    point2d<decl_mul_t<T, U>>
> operator * (const point2d<T>& left, const U& right) noexcept
{
    return { as_vec2(left) * right };
}

template<class U, class T>
[[nodiscard]] constexpr auto operator * (const U& left, const point2d<T>& right) noexcept -> decltype(right* left)
{
    return right * left;
}

template<class T, class U>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_arithmetic_v<U>,
    point2d<decl_div_t<T, U>>
> operator / (const point2d<T>& left, const U& right) noexcept
{
    return { as_vec2(left) / right };
}
