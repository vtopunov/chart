#include <widget/run.h>
#include <widget/button.h>

#include <chart/space.h>
#include <chart/background.h>
#include <chart/polyline.h>
#include <chart/grid.h>
#include <chart/value_labels.h>


namespace
{
    constexpr auto n_points = 1000_uz;

    constexpr auto abscissa = [] () noexcept
    {
        constexpr auto pi = 3.141592653589793238462643383279502884L;
        constexpr auto abscissa_max = static_cast<real_t>(7.0 * pi);
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
        static constexpr rectangle button_geometry
        {
            point2d
            {
                narrow<npx_t>(20_npx + n * (5_npx + button_width)),
                D_OS_ANDROID_OR(60_npx, 20_npx)
            },
            size2d
            {
                button_width,
                D_OS_ANDROID_OR(60_npx, 40_npx)
            }
        };

        widget::button b_plot
        {
            .geometry{ button_geometry<0u> },
            .text{ u8"Построить" }
        };

        widget::button b_clear
        {
            .geometry{ button_geometry<1u> },
            .text{ u8"Очистить" }
        };

        widget::button b_exit
        {
            .geometry{ button_geometry<2u> },
            .text{ u8"Выход" }
        };

        struct chart_widget
        {
            static constexpr rectangle chart_geometry
            {
                point2d{ 8_npx, button_geometry<0>.y1() + 15_npx },
                size2d{ -20_pxoff, -20_pxoff }
            };

            chart::space space
            {
                .geometry{ chart_geometry },
            };

            static constexpr chart::background background{ .brush{ colors::yellow_f.with_blue(0.93f) } };
            static constexpr chart::grid grid{};

            //chart::px_grid px_grid{};
            chart::value_labels labels{};

            struct polyline : chart::polyspanline
            {
                chart::vpoint2re points;

                void setup_model() noexcept
                {
                    set_model(std::as_const(points));
                }
            };

            polyline polyline0{ {.pen{colors::red_f } } };
            polyline polyline1{ {.pen{colors::blue_f} } };

            bool operator () (widget::basic_initialization_event<>) noexcept
            {
                if (polyline0.points.try_reserve(n_points) && polyline1.points.try_reserve(n_points)) [[likely]]
                {
                    for (size_t i = 0u; i < n_points; ++i)
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
                return fn(space(background, grid, labels, polyline0, polyline1));
            }
        };

        chart_widget chart{};

        main_widget() noexcept
        {
            b_plot.clicked = [this] () noexcept
            {
                chart.plot();
            };

            b_clear.clicked = [this] () noexcept
            {
                chart.clear();
            };

            b_exit.clicked = [] () noexcept
            {
                ui::quit();
            };
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(b_plot, b_clear, b_exit, chart);
        }
    };
}


int main() noexcept
{
    return widget::run<main_widget>();
}