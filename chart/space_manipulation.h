#pragma once

#include <core/transformation.h>

#include <chart/fwd.h>


namespace chart
{
    using space_diagonal = vec2<real_point2d>;
    using space_manipulation = vec2<space_diagonal>;
    using space_transformation = decltype(::make_transformation(std::declval<const space_manipulation&>()));
    using space_scale_transformation = decltype(::make_scale_transformation(std::declval<const space_manipulation&>()));

    constexpr space_diagonal space_diagonal_initializer
    {
        real_point2d_inf,
        real_point2d_lowest_inf
    };

    [[nodiscard]]
    constexpr space_diagonal make_pxspace_diagonal(pxsizes sizes) noexcept
    {
        constexpr real_t real_zero{ zero_v<> };
        D_ASSERT(sizes.has_positive_square());

        const auto [x, y] = md_narrow<real_point2d>(sizes);

        return
        {
            { real_zero, y - 1.0 },
            { x - 1.0, real_zero },
        };
    }
}