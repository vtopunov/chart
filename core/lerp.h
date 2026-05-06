#pragma once

#include <core/math.h>


template<class A1, class A0 = A1>
struct polynomial2
{
    A1 a1;
    A0 a0;

    template<class Arg>
    [[nodiscard]] constexpr auto operator () (const Arg& argument) const noexcept
        -> decltype(a1* argument + a0)
    {
        return a1 * argument + a0;
    }

    [[nodiscard]]
    constexpr bool operator == (const polynomial2&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const polynomial2&) const noexcept = default;
};

template<class A1>
struct polynomial2<A1, void>
{
    A1 a1;

    template<class Arg>
    [[nodiscard]] constexpr auto operator () (const Arg& argument) const noexcept
        -> decltype(a1 * argument)
    {
        return a1 * argument;
    }

    [[nodiscard]]
    constexpr bool operator == (const polynomial2&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const polynomial2&) const noexcept = default;
};

template<class A1, class A0>
polynomial2(A1, A0) -> polynomial2<A1, A0>;

template<class A1>
polynomial2(A1) -> polynomial2<A1, void>;



template<class A0, class A1, class V0, class V1>
[[nodiscard]] constexpr decltype(auto) lerp_scale_value(const A0& arg0, const A1& arg1, const V0& value0, const V1& value1) noexcept
{
    D_ASSERT(::is_neqn(arg0, arg1));
    return (value1 - value0) / (arg1 - arg0);
}

template<class A0, class A1, class V0, class V1>
[[nodiscard]] constexpr decltype(auto) lerp_shift_value(const A0& arg0, const A1& arg1, const V0& value0, const V1& value1) noexcept
{
    D_ASSERT(::is_neqn(arg0, arg1));
    return (value0 * arg1 - value1 * arg0) / (arg1 - arg0);
}

constexpr struct
{
    template<class A0, class A1, class V0, class V1>
    [[nodiscard]] constexpr decltype(auto) operator () (const A0& arg0, const A1& arg1, const V0& value0, const V1& value1) const noexcept
    {
        return polynomial2
        {
            lerp_scale_value(arg0, arg1, value0, value1),
            lerp_shift_value(arg0, arg1, value0, value1)
        };
    }
} lerp{};


constexpr struct
{
    template<class A0, class A1, class V0, class V1>
    [[nodiscard]] constexpr decltype(auto) operator () (const A0& arg0, const A1& arg1, const V0& value0, const V1& value1) const noexcept
    {
        return polynomial2{ lerp_scale_value(arg0, arg1, value0, value1) };
    }
} lerp_scale{};