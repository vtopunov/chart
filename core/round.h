#pragma once

#include <cmath>

#include <core/clamp_cast.h>


template<class OutT, class InT>
OutT round_cast(InT in) noexcept
{
    if constexpr (std::is_floating_point_v<InT>)
    {
        constexpr auto out_digits = numeric_digits_v<OutT>;
        constexpr auto l_digits = numeric_digits_v<long>;

        if constexpr (out_digits > l_digits)
        {
            return clamp_cast<OutT>(std::llround(in));
        }
        else
        {
            return clamp_cast<OutT>(std::lround(in));
        }
    }
    else
    {
        return clamp_cast<OutT>(in);
    }
}

template<class OutT, class InT>
OutT trunc_cast(InT in) noexcept
{
    if constexpr (std::is_floating_point_v<InT>)
    {
        return clamp_cast<OutT>(std::trunc(in));
    }
    else
    {
        return clamp_cast<OutT>(in);
    }
}