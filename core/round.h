#pragma once

#include <core/zero.h>
#include <core/clamp_cast.h>


namespace private_detail_round
{
    constexpr struct
    {
        template<class Source>
        [[nodiscard]] constexpr decltype(auto) operator () (Source v) const noexcept
        {
            return std::round(v);
        }
    } round_fn_v{};

    constexpr struct
    {
        template<class Source>
        [[nodiscard]] constexpr decltype(auto) operator () (Source v) const noexcept
        {
            return std::ceil(v);
        }
    } ceil_fn_v{};

    constexpr struct
    {
        template<class Source>
        [[nodiscard]] constexpr decltype(auto) operator () (Source v) const noexcept
        {
            return std::floor(v);
        }
    } floor_fn_v{};

    constexpr struct
    {
        template<class Source>
        [[nodiscard]] constexpr decltype(auto) operator () (Source v) const noexcept
        {
            return std::trunc(v);
        }
    } trunc_fn_v{};


    template<class Target, class Source>
    [[nodiscard]] constexpr Target round_cast(Source v) noexcept
    {
        return clamp_cast<Target>(v, round_fn_v);
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target ceil_cast(Source v) noexcept
    {
        return clamp_cast<Target>(v, ceil_fn_v);
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target floor_cast(Source v) noexcept
    {
        return clamp_cast<Target>(v, floor_fn_v);
    }

    template<class Target, class Source>
    [[nodiscard]] constexpr Target trunc_cast(Source v) noexcept
    {
        return clamp_cast<Target>(v, trunc_fn_v);
    }
}

using private_detail_round::round_cast;
using private_detail_round::ceil_cast;
using private_detail_round::floor_cast;
using private_detail_round::trunc_cast;


template<class T, class Near>
[[nodiscard]] constexpr std::enable_if_t<
    std::negation_v<std::conjunction<std::is_floating_point<T>, std::is_integral<Near>>>,
    Near
> round_to_near(T value, Near) noexcept
{
    return clamp_cast<Near>(value);
}

template<class T, class Near>
[[nodiscard]] constexpr std::enable_if_t<
    std::conjunction_v<std::is_floating_point<T>, std::is_integral<Near>>,
    Near
> round_to_near(T value, Near near_value) noexcept
{
    constexpr Near one{ 1 };

    const auto floor_value = floor_cast<Near>(value);
    return (near_value <= floor_value) ? (floor_value) : (floor_value + one);
}


template<class T>
[[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, bool> u_isnormal(const T& value) noexcept
{
    return std::isnormal(value);
}

template<class T>
[[nodiscard]] constexpr auto u_isnormal(const T& value) noexcept -> std::enable_if_t<
    std::negation_v<std::is_floating_point<T>>,
    decltype(is_neqz(value))
>
{
    return is_neqz(value);
}


template<class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::conjunction_v<std::negation<std::is_floating_point<T>>, has_pre_inc_op<T>>,
    T
> u_next(T value) noexcept
{
    return ++value;
}

template<class T>
[[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, T> u_next(T value) noexcept
{
    return std::nextafter(value, std::numeric_limits<T>::infinity());
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<
    std::conjunction_v<std::negation<std::is_floating_point<T>>, has_pre_dec_op<T>>,
    T
> u_prev(T value) noexcept
{
    return --value;
}

template<class T>
[[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, T> u_prev(T value) noexcept
{
    return std::nextafter(value, -std::numeric_limits<T>::infinity());
}