#pragma once

#include <core/vec.h>

template<class T>
struct polynomial
{
    using coefficients_type = T;

    vec<coefficients_type> coefficients;

    template<class Arg>
    constexpr decltype(auto) operator () (const Arg& argument) const noexcept
    {
        return coefficients._1 * argument + coefficients._0;
    }
};

template<class T>
polynomial(T, T)->polynomial<T>;