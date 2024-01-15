#include <widget/run.h>
#include <widget/button.h>

#include <chart/chart_widget.h>


namespace
{
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

        chart_widget chart
        {
            .geometry{ chart_boundaries }
        };

        chart_line::points_type points{};

        bool operator () (os::const_module_handle_t app) noexcept
        {
            const auto initialize_container_of_points = [&v = points] () noexcept
            {
                constexpr auto n_points = 1000_uz;

                if (!v.try_reserve(n_points)) [[unlikely]]
                {
                    e_debug("chart_widget: initialize_container_of_points: out of memory");
                    return false;
                }

                constexpr auto pi = 3.141592653589793238462643383279502884L;
                constexpr auto abscissa_max = static_cast<px::real_t>(12.0 * pi);
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
            };

            if (!initialize_container_of_points()) [[unlikely]]
            {
                return false;
            };

            b_plot.clicked = [this] () noexcept
            {
                if (!chart.line.points.size())
                {
                    chart.line.points = std::move(points);
                }

                chart.clear_cache();
            };

            b_clear.clicked = [this] () noexcept
            {
                if (chart.line.points.size())
                {
                    points = std::move(chart.line.points);
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