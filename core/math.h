#pragma once

#include <cmath>
#include <utility>
#include <numeric>

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

template<class L, class M, class R> 
[[nodiscard]] constexpr auto is_less_mid(L left, M mid, R right) noexcept -> decltype
(
    less_op(left, mid) && less_op(mid, right)
)
{
    return less_op(left, mid) && less_op(mid, right);
}


template<class L, class R = L>
[[nodiscard]] constexpr auto is_less_midp(L left, R right) noexcept -> decltype
(
    is_less_mid(left, std::midpoint<std::common_type_t<L, R>>(left, right), right)
)
{
    return is_less_mid(left, std::midpoint<std::common_type_t<L, R>>(left, right), right);
}

template<class L, class R = L>
[[nodiscard]] constexpr auto is_less_neqfp(L left, R right) noexcept -> decltype
(
    less_op(left, right) && is_less_midp(left, right)
)
{
    return less_op(left, right) && is_less_midp(left, right);
}

template<class L, class R = L>
[[nodiscard]] constexpr auto is_greater_neqfp(L left, R right) noexcept -> decltype(is_less_neqfp(right, left))
{
    return is_less_neqfp(right, left);
}

template<class L, class R = L>
[[nodiscard]] constexpr auto is_neqfp(L left, R right) noexcept -> decltype
(
    is_less_neqfp(left, right) || is_greater_neqfp(left, right)
)
{
    const auto mid = std::midpoint<std::common_type_t<L, R>>(left, right);
    return is_less_mid(left, mid, right) || is_less_mid(right, mid, left);
}

template<class L, class R = L>
[[nodiscard]] constexpr auto is_eqfp(L left, R right) noexcept -> decltype(!is_neqfp(left, right))
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


template<class L, class R = L>
[[nodiscard]] constexpr auto is_neqn(const L& left, const R& right) noexcept -> typename enable_if_detected_or<enable_if_detected<
    decl_n_op_eq_op_t, L, R>, 
    decl_neq_op_t, L, R>::
    type
{
    if constexpr (std::disjunction_v<std::is_floating_point<L>, std::is_floating_point<R> >)
    {
        return is_neqfp(left, right);
    }
    else
    {
        if constexpr (std::conjunction_v<std::is_integral<L>, std::is_integral<R> >)
        {
            return std::cmp_not_equal(left, right);
        }
        else
        {
            if constexpr (is_detected_v<decl_neq_op_t, L, R>)
            {
                return left != right;
            }
            else
            {
                return !(left == right);
            }
        }
    }
}