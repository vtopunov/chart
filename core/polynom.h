#pragma once

#include "vec.h"

template<class CoefficientsType, class ResultType = CoefficientsType, class ArgumentType = CoefficientsType>
struct polynom
{
    using coefficients_type = CoefficientsType;
    using result_type = ResultType;
    using argument_type = ArgumentType;

    vec<CoefficientsType> coefficients{};

    constexpr polynom() noexcept = default;

    constexpr polynom( CoefficientsType coefficient0, CoefficientsType coefficient1 ) noexcept
        : coefficients{ coefficient0, coefficient1 }
    {}

    constexpr result_type operator () (argument_type argument) const noexcept
    {
        return narrow_cast<result_type>( coefficients._1 * argument + coefficients._0 );
    }
};

template<class ResultType, class ArgumentType, class CoefficientsType>
constexpr polynom<CoefficientsType, ResultType, ArgumentType> make_polynom( CoefficientsType coefficient0, CoefficientsType coefficient1 ) noexcept
{
    return { coefficient0, coefficient1 };
}