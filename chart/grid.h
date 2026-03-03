#pragma once

#include <px/algorithm.h>

#include <chart/periodic_position.h>
#include <chart/grid_shader.h>
#include <chart/space_manipulation.h>


namespace chart
{
    template<class T>
    [[nodiscard]] constexpr T grid_increment(const T min_distance) noexcept
    {
        const auto max_increment = pow(10, ceil_cast<int64_t>(log10(min_distance)));
        const auto half_increment = 0.5 * max_increment;
        const auto result_increment = (min_distance <= half_increment) ? half_increment : max_increment;
        D_ASSERT_OR_ASSUME(min_distance <= result_increment);
        return result_increment;
    }

    template<class T>
    [[nodiscard]] constexpr T grid_begin(T begin, T increment) noexcept
    {
        const auto result = increment * std::ceil(begin / increment);
        D_ASSERT_OR_ASSUME(begin <= result);
        D_ASSERT_OR_ASSUME(result <= (begin + increment));
        return result;
    }

    template<class T>
    [[nodiscard]] constexpr size_t periodic_count(T begin, T increment, T end) noexcept
    {
        const auto last = (end - begin) / increment;

        {
            [[maybe_unused]] constexpr auto index_epsf = numeric_eps_v<T>;
            [[maybe_unused]] constexpr auto index_minf = clamp_cast<T>(-1) + index_epsf;
            [[maybe_unused]] constexpr auto index_maxf = clamp_cast<T>(size_overflow_maxi) - index_epsf;
            D_ASSERT_OR_ASSUME(last >= index_minf);
            D_ASSERT_OR_ASSUME(last <= index_maxf);
        }

        return 1u + static_cast<size_t>(last);
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

    template<class T>
    [[nodiscard]] constexpr point2d<size_t> md_periodic_count(point2d<T> begin, point2d<T> increment, point2d<T> end) noexcept
    {
        return
        {
            periodic_count(begin._0, increment._0, end._0),
            periodic_count(begin._1, increment._1, end._1)
        };
    }

    template<class T>
    struct basic_grid;

    struct shader_grid_drawer
    {
        using cache_type = dummy;

        inline static void draw(const basic_grid<shader_grid_drawer>& grid, const periodic_value_position& position, const shader::grid& shdr) noexcept;
    };

    struct px_grid_drawer
    {
        struct cache_type
        {
            gl::unique_texture2d_resource texture{};
        };

        inline static void draw(const basic_grid<px_grid_drawer>& grid, const periodic_value_position& position, const lumpixspan pixs) noexcept;

        inline static void draw(const basic_grid<px_grid_drawer>& grid, const shader_embed::luminance_texture& shdr) noexcept;
    };

    template<class Drawer>
    struct basic_grid
    {
        using cache_type = typename Drawer::cache_type;

        static constexpr auto default_color = ::colors::green_f;
        static constexpr auto default_widths = fill_to<point2d>(1_npx);
        static constexpr point2d default_min_distances{ 50_npx, 30_npx };

        rgbaf_color color{ default_color };
        pxpoint widths{ default_widths };
        pxpoint min_distances{ default_min_distances };
        D_NO_UNIQUE_ADDRESS cache_type cache{};


        [[nodiscard]]
        constexpr periodic_value_position operator () (const space_manipulation& sys) const noexcept
        {
            const auto abs_scale_to_px = md_abs(make_scale_transformation(sys).scale());
            const auto math_repeat = md_grid_increment(min_distances / abs_scale_to_px);
            const auto math_begin = md_grid_begin(sys._0._0, math_repeat);

            return
            {
                .value{.begin{ math_begin }, .repeat{ math_repeat } },
                .px
                {
                    .begin{ abs_scale_to_px * (math_begin - sys._0._0) },
                    .repeat{ abs_scale_to_px * math_repeat }
                },
                .count{ md_periodic_count(math_begin, math_repeat, sys._0._1) }
            };
        }


        template<class... Args>
        auto operator () (const Args&... args) const noexcept -> decltype(Drawer::draw(*this, args...))
        {
            return Drawer::draw(*this, args...);
        }
    };

    inline void shader_grid_drawer::draw(const basic_grid<shader_grid_drawer>& grid, const periodic_value_position& position, const shader::grid& shdr) noexcept
    {
        shdr.color(grid.color);
        shdr.width(grid.widths);
        shdr.begin(position.px.begin);
        shdr.repeat(position.px.repeat);
        shdr.draw();
    }

    inline void px_grid_drawer::draw(const basic_grid<px_grid_drawer>& grid, const periodic_value_position& position, const lumpixspan pixs) noexcept
    {
        zero_memory(pixs);

        for (size_t i = 0; i != position.count.y(); ++i )
        {
            const auto row_index = position.px.begin.y() + i * position.px.repeat.y();
            px::draw_hline(pixs, row_index, grid.widths.y());
        } 

        for (size_t i = 0; i != position.count.x(); ++i )
        {
            const auto column_index = position.px.begin.x() + i * position.px.repeat.x();
            px::draw_vline(pixs, column_index, grid.widths.x());
        }

        {
            const auto result = gl::update(as_mutable(grid.cache.texture), pixs);
            D_ASSERT_OR_ASSUME(result);
        }
    }

    inline void px_grid_drawer::draw(const basic_grid<px_grid_drawer>& grid, const shader_embed::luminance_texture& shdr) noexcept
    {
        shdr.color(grid.color);
        shdr.texture(grid.cache.texture);
        shdr.draw();
    }

    using grid = basic_grid<shader_grid_drawer>;
    using px_grid = basic_grid<px_grid_drawer>;
}
