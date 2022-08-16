#pragma once

#include <core/vec2.h>


template<class T>
struct polynomial2
{
    using coefficients_type = T;

    vec2<coefficients_type> coefficients;

    template<class Arg>
    [[nodiscard]] constexpr decltype(auto) operator () (const Arg& argument) const noexcept
    {
        return coefficients._1 * argument + coefficients._0;
    }
};

template<class T>
polynomial2(T, T)->polynomial2<T>;
