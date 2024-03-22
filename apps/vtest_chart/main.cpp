#include <widget/run.h>
#include <widget/button.h>

#include <chart/space.h>
#include <chart/background.h>
#include <chart/grid.h>
#include <chart/polyline.h>


namespace
{
    constexpr auto n_points = 1000_uz;

    constexpr auto abscissa = [] () noexcept
    {
        constexpr auto pi = 3.141592653589793238462643383279502884L;
        constexpr auto abscissa_max = static_cast<px::real_t>(7.0 * pi);
        return lerp
        (
            0_uz, n_points - 1_uz,
            -abscissa_max, abscissa_max
        );
    }();


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

        struct chart_widget
        {
            static constexpr rectangle chart_boundaries
            {
                button_boundaries_v<0>.p01() + point2d{0_npx, 15_npx},
                size2d{ -20_npxz, -20_npxz }
            };

            chart::space space
            {
                .geometry{ chart_boundaries },
            };

            static constexpr chart::background background{ .color{ colors::yellow_f.with_blue(0.93f) }};
            static constexpr chart::grid grid{};

            struct polyline : chart::polyspanline
            {
                chart::real_vpoint2d points;

                void setup_model() noexcept
                {
                    set_model(std::as_const(points));
                }
            };

            polyline polyline0{ {.pen_color{colors::red_f } } };
            polyline polyline1{ {.pen_color{colors::blue_f} } };

            bool operator () (widget::basic_initialization_event<>) noexcept
            {
                if (polyline0.points.try_reserve(n_points) && polyline1.points.try_reserve(n_points)) [[likely]]
                {
                    for (size_t i = 0_uz; i < n_points; ++i)
                    {
                        const auto x = abscissa(i);
                        const auto y = sin(x);
                        polyline0.points.emplace_back(x, y);
                        polyline1.points.emplace_back(x, 0.95 * y);

                    }

                    plot();

                    return true;
                }

                e_debug("chart: out of memory");
                return false;
            }

            void plot() noexcept
            {
                polyline0.setup_model();
                polyline1.setup_model();
                space.clear_cache();
            }

            void clear() noexcept
            {
                polyline0.reset_model();
                polyline1.reset_model();
                space.clear_cache();
            }

            template<class Fn>
            decltype(auto) apply(Fn fn) noexcept
            {
                return fn(space(background, grid, polyline0, polyline1));
            }
        };

        chart_widget chart{};


        bool operator () (widget::initialization_event<> e) noexcept
        {
            b_plot.clicked = [this] () noexcept
            {
                chart.plot();
            };

            b_clear.clicked = [this] () noexcept
            {
                chart.clear();
            };

            b_exit.clicked = [app = e.app()] () noexcept
            {
                ui::quit(app);
            };

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