#pragma once

#include <cmath>

#include <core/limits.h>


template<class T>
[[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, T> prevfp(T value) noexcept
{
    return std::nextafter(value, numeric_lowest_inf_v<T>);
}

template<class T>
[[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, T> nextfp(T value) noexcept
{
    return std::nextafter(value, numeric_inf_v<T>);
}

template<class T>
[[nodiscard]] constexpr auto is_less_neqfp(T left, T right) noexcept -> decltype
(
    less_op(left, right) && less_op(nextfp(left), right)
)
{
    return less_op(left, right) && less_op(nextfp(left), right);
}

template<class T>
[[nodiscard]] constexpr auto is_greater_neqfp(T left, T right) noexcept -> decltype(is_less_neqfp(right, left))
{
    return is_less_neqfp(right, left);
}

template<class T>
[[nodiscard]] constexpr auto is_neqfp(T left, T right) noexcept -> decltype
(
    is_less_neqfp(left, right) || is_greater_neqfp(left, right)
)
{
    return is_less_neqfp(left, right) || is_greater_neqfp(left, right);
}

template<class T>
[[nodiscard]] constexpr auto is_eqfp(T left, T right) noexcept -> decltype(!is_neqfp(left, right))
{
    return !is_neqfp(left, right);
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_floating_point_v<T>, T> is_negative_nepsfp(T value) noexcept
{
    return less_op(value, numeric_lowest_eps_v<T>);
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_floating_point_v<T>, T> is_positive_nepsfp(T value) noexcept
{
    return less_op(numeric_eps_v<T>, value);
}

template<class T>
[[nodiscard]] constexpr auto is_positive_or_epsfp(T value) noexcept -> decltype(!is_negative_nepsfp(value))
{
    return !is_negative_nepsfp(value);
}

template<class T>
[[nodiscard]] constexpr auto is_negative_or_epsfp(T value) noexcept -> decltype(!is_positive_nepsfp(value))
{
    return !is_positive_nepsfp(value);
}
