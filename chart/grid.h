#pragma once

#include <chart/grid_shader.h>
#include <chart/space_manipulation.h>


namespace chart
{
    struct periodic_position
    {
        px::real_point2d begin;
        px::real_point2d repeat;
    };

    struct periodic_value_position
    {
        periodic_position value;
        periodic_position px;
    };

    template<class T>
    [[nodiscard]] constexpr T grid_increment(const T min_distance) noexcept
    {
        const auto max_increment = pow(10, ceil_cast<int64_t>(log10(min_distance)));
        const auto half_increment = 0.5 * max_increment;
        const auto result_increment = (min_distance <= half_increment) ? half_increment : max_increment;
        D_ASSERT(min_distance <= result_increment); // c++26+ contracts
        return result_increment;
    }

    template<class T>
    [[nodiscard]] constexpr T grid_begin(T begin, T increment) noexcept
    {
        const auto result = increment * std::ceil(begin / increment);
        D_ASSERT(begin <= result); // c++26+ contracts
        D_ASSERT(result <= (begin + increment));
        return result;
    }

    template<class T>
    [[nodiscard]] constexpr point2d<T> md_grid_increment(point2d<T> min_distances) noexcept
    {
        return
        {
            grid_increment(min_distances._0),
            grid_increment(min_distances._1),
        };
    }

    template<class T>
    [[nodiscard]] constexpr point2d<T> md_grid_begin(point2d<T> begin, point2d<T> increment) noexcept
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

        rgbaf_color color{ default_color };
        pxpoint2d widths{ default_widths };
        pxpoint2d min_distances{ default_min_distances };

        constexpr periodic_value_position operator () (const space_manipulation& sys) const noexcept
        {
            const auto abs_scale_to_px = md_abs(make_scale_transformation(sys).scale());
            const auto math_repeat = md_grid_increment(min_distances / abs_scale_to_px);
            const auto repeat = abs_scale_to_px * math_repeat;
            const auto math_begin = md_grid_begin(sys._0._0, math_repeat);
            const auto begin = abs_scale_to_px * (math_begin - sys._0._0);

            return
            {
                .value{.begin{ math_begin }, .repeat{ math_repeat } },
                .px{.begin{ begin }, .repeat{ repeat } }
            };
        }

        void operator () (const shader::grid_user& shdr, const periodic_value_position& position) const noexcept
        {
            shdr.color(color)
                .width(widths)
                .begin(position.px.begin)
                .repeat(position.px.repeat)
                .draw();
        }
    };

}
