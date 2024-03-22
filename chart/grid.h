#pragma once

#include <chart/grid_shader.h>


namespace chart
{
    template<class T>
    [[nodiscard]] T grid_increment(const T min_distance) noexcept
    {
        const auto max_increment = pow(10, ceil_cast<int64_t>(log10(min_distance)));
        const auto half_increment = 0.5 * max_increment;
        const auto result_increment = (min_distance <= half_increment) ? half_increment : max_increment;
        D_ASSERT(min_distance <= result_increment); // c++26+ contracts
        return result_increment;
    }

    template<class T>
    [[nodiscard]] T grid_begin(T begin, T increment) noexcept
    {
        const auto result = increment * std::ceil(begin / increment);
        D_ASSERT(begin <= result); // c++26+ contracts
        D_ASSERT(result <= (begin + increment));
        return result;
    }

    template<class T>
    [[nodiscard]] point2d<T> md_grid_increment(point2d<T> min_distances) noexcept
    {
        return
        {
            grid_increment(min_distances._0),
            grid_increment(min_distances._1),
        };
    }

    template<class T>
    [[nodiscard]] point2d<T> md_grid_begin(point2d<T> begin, point2d<T> increment) noexcept
    {
        return
        {
            grid_begin(begin._0, increment._0),
            grid_begin(begin._1, increment._1)
        };
    }


    struct grid
    {
        static constexpr auto default_color = ::colors::green_f;
        static constexpr auto default_widths = fill_to<point2d>(1_npx);
        static constexpr point2d default_min_distances{ 50_npx, 30_npx };

        rgbaf_color_t color{ default_color };
        pxpoint2d widths{ default_widths };
        pxpoint2d min_distances{ default_min_distances };

        void operator () (const shader::grid_user& shdr, const space_diagonal_t math_space, const transformation_t math2px) const noexcept
        {
            const auto abs_math2px_scale = md_abs(math2px.scale());
            const auto math_repeat = md_grid_increment(min_distances / abs_math2px_scale);
            const auto repeat = abs_math2px_scale * math_repeat;
            const auto math_begin = md_grid_begin(math_space._0, math_repeat);
            const auto begin = abs_math2px_scale * (math_begin - math_space._0);

            shdr.color(color)
                .width(widths)
                .begin(begin)
                .repeat(repeat)
                .draw();
        }
    };

}
