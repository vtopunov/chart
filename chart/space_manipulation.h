#pragma once

#include <core/transformation.h>

#include <chart/fwd.h>


namespace chart
{
    using space_diagonal = vec2<point2re>;
    using space_manipulation = vec2<space_diagonal>;
    using space_transformation = decltype(::make_transformation(std::declval<const space_manipulation&>()));
    using space_scale_transformation = decltype(::make_scale_transformation(std::declval<const space_manipulation&>()));

    constexpr space_diagonal space_diagonal_initializer
    {
        fill_to<point2re>(numeric_inf_v<>),
        fill_to<point2re>(numeric_lowest_inf_v<>)
    };

    [[nodiscard]]
    constexpr space_diagonal make_space_diagonal(pxsizes sizes) noexcept
    {
        constexpr real_t zero_re{ zero_v<> };
        constexpr auto back = fill_to<size2d>(1_npx);
        D_ASSERT(md_is_positiven(sizes));

        const auto [space_x, space_y] = md_narrow<point2re>(sizes - back);

        return
        {
            point2re{ zero_re, space_y },
            point2re{ space_x, zero_re },
        };
    }
}