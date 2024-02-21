#include <widget/run.h>
#include <widget/button.h>

#include <chart/space.h>
#include <chart/background.h>

namespace
{
    [[nodiscard]] bool initialize_container_of_points(chart::real_vpoint2d& v) noexcept
    {
        constexpr auto n_points = 1000_uz;

        if (!v.try_reserve(n_points)) [[unlikely]]
        {
            e_debug("chart: initialize_container_of_points: out of memory");
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

    [[nodiscard]] bool initialize_second_container_of_points(chart::real_vpoint2d& v, chart::real_point2d_cspan points) noexcept
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
        static constexpr auto button_width = 120_npx;

        template<size_t n>
        static constexpr rectangle button_boundaries_v
        {
            point2d
            {
                narrow<pxsize_t>(20_npx + n * (5_npx + button_width)),
                D_CONDITIONAL_OS_ANDROID(60_npx, 20_npx)
            },
            size2d
            {
                button_width,
                D_CONDITIONAL_OS_ANDROID(60_npx, 40_npx)
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
            button_boundaries_v<0>.p01() + point2d{0_npx, 15_npx},
            size2d{ -20_npxz, -20_npxz }
        };

        chart::space chart_space
        {
            .geometry{ chart_boundaries },
        };

        static constexpr chart::background chart_background{ .color{ colors::yellow_f.with_blue(0.85f) } };

        struct tunable_polyline : chart::polyspanline
        {
            chart::real_vpoint2d points;

            void setup() noexcept
            {
                set_model(std::as_const(points));
            }
        };

        tunable_polyline polyline0{ {.pen_color{colors::red_f } } };
        tunable_polyline polyline1{ {.pen_color{colors::blue_f} } };

        bool operator () (os::const_module_handle_t app) noexcept
        {
            if (!initialize_container_of_points(polyline0.points)) [[unlikely]]
            {
                return false;
            };

            if (!initialize_second_container_of_points(polyline1.points, polyline0.points)) [[unlikely]]
            {
                return false;
            };

            b_plot.clicked = [this] () noexcept
            {
                polyline0.setup();
                polyline1.setup();
                chart_space.clear_cache();
            };

            b_clear.clicked = [this] () noexcept
            {
                polyline0.clear();
                polyline1.clear();
                chart_space.clear_cache();
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
            return fn(b_plot, b_clear, b_exit, chart_space(chart_background, polyline0, polyline1));
        }
    };
}


int app_main(os::module_handle_t app) noexcept
{
    return widget::run<main_widget>(app);
}