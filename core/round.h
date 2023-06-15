#pragma once

#include <cmath>

#include <core/clamp_cast.h>


namespace private_detail_round_cast
{
    template<class Target>
    struct round_fn
    {
        template<class Source>
        constexpr Target operator () (Source v) const noexcept
        {
            return static_cast<Target>(std::round(v));
        }
    };

    template<class Target>
    struct trunc_fn
    {
        template<class Source>
        constexpr Target operator () (Source v) const noexcept
        {
            return static_cast<Target>(std::trunc(v));
        }
    };

    template<class Target, class Source, class Fn>
    [[nodiscard]] Target round_cast_impl(Source v, Fn fn) noexcept
    {
        using source_t = std::remove_cvref_t<Source>;

        if constexpr (std::is_floating_point_v<source_t>)
        {
            if constexpr (std::is_integral_v<Target>)
            {
                return private_detail_clamp_cast::clamp_minmax_cast<Target>(v, fn);
            }
            else
            {
                return fn(v);
            }
        }
        else
        {
            return clamp_cast<Target>(v);
        }
    }

    template<class Target, class Source>
    [[nodiscard]] Target round_cast(Source v) noexcept
    {
        constexpr round_fn<Target> fn{};
        return round_cast_impl<Target>(v, fn);
    }

    template<class Target, class Source>
    [[nodiscard]] Target trunc_cast(Source v) noexcept
    {
        constexpr trunc_fn<Target> fn{};
        return round_cast_impl<Target>(v, fn);
    }
}

using private_detail_round_cast::round_cast;
using private_detail_round_cast::trunc_cast;