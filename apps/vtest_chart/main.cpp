#include <widget/run.h>
#include <widget/button.h>

#include <chart/chart_widget.h>


namespace
{
    bool initialize_container_of_points(chart_line::points_type& v) noexcept
    {
        constexpr auto n_points = 1000_uz;

        if (!v.try_reserve(n_points)) [[unlikely]]
        {
            e_debug("chart_widget: initialize_container_of_points: out of memory");
            return false;
        }

        constexpr auto pi = 3.141592653589793238462643383279502884L;
        constexpr auto abscissa_max = static_cast<px::real_t>(7.0 * pi);
        constexpr auto abscissa = lerp
        (
            0_uz, n_points - 1_uz,
            -abscissa_max, abscissa_max
        );

        D_ASSERT(0u == v.size());
        D_ASSERT(n_points <= v.capacity());
        for (size_t i = 0_uz; i < n_points; ++i)
        {
            const auto x = abscissa(i);
            v.emplace_back(x, sin(x));
        }

        return true;
    }

    bool initialize_second_container_of_points(chart_line::points_type& v, chart::real_point2d_cspan points) noexcept
    {
        if (!v.try_reserve(points.size())) [[unlikely]]
        {
            e_debug("chart_widget: initialize_second_container_of_points: out of memory");
            return false;
        }

        D_ASSERT(0u == v.size());
        D_ASSERT(points.size() <= v.capacity());
        for (const auto& p : points)
        {
            v.emplace_back(p.x(), 0.95 * p.y());
        }

        return true;
    }

    struct main_widget
    {
        static constexpr auto button_width = 120_px;

        template<size_t n>
        static constexpr rectangle button_boundaries_v
        {
            point2d
            {
                narrow<pxside_t>(20_px + n * (5_px + button_width)),
                D_CONDITIONAL_OS_ANDROID(60_px, 20_px)
            },
            size2d
            {
                button_width,
                D_CONDITIONAL_OS_ANDROID(60_px, 40_px)
            }
        };

        widget::button b_plot
        {
            .geometry{ button_boundaries_v<0u> },
            .text{ u8"Построить" }
        };

        widget::button b_clear
        {
            .geometry{ button_boundaries_v<1u> },
            .text{ u8"Очистить" }
        };

        widget::button b_exit
        {
            .geometry{ button_boundaries_v<2u> },
            .text{ u8"Выход" }
        };

        static constexpr rectangle chart_boundaries
        {
            button_boundaries_v<0>.p01() + point2d{0_px, 15_px},
            size2d{ -20_pxz, -20_pxz }
        };

        chart_line chart_red_line{ .pen_color{ gl::colors::red_f } };
        chart_line chart_blue_line{ .pen_color{ gl::colors::blue_f } };

        chart_widget chart
        {
            .geometry{ chart_boundaries },
        };


        bool operator () (os::const_module_handle_t app) noexcept
        {
            if (!initialize_container_of_points(chart_red_line.points)) [[unlikely]]
            {
                return false;
            };

            if (!initialize_second_container_of_points(chart_blue_line.points, chart_red_line.points)) [[unlikely]]
            {
                return false;
            };

            b_plot.clicked = [this] () noexcept
            {
                if (chart.lines.is_empty())
                {
                    chart.lines.attach_to_back(&chart_red_line);
                    chart.lines.attach_to_back(&chart_blue_line);
                    chart.clear_cache();
                }
            };

            b_clear.clicked = [this] () noexcept
            {
                if (!chart.lines.is_empty())
                {
                    chart.lines.clear();
                    chart.clear_cache();
                }
            };

            b_exit.clicked = [app] () noexcept
            {
                ui::quit(app);
            };

            b_plot.clicked();

            return true;
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(b_plot, b_clear, b_exit, chart);
        }
    };
}


int app_main(os::module_handle_t app) noexcept
{
    return widget::run<main_widget>(app);
}