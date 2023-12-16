#include <core/small_vector.h>

#include <px/algorithm.h>

#include <utility/px.h>

#include <widget/run.h>
#include <widget/button.h>
#include <widget/stretchable.h>

#include <chart/space_diagonal.h>


using namespace std::string_view_literals;

using widget::event_result;


namespace
{
    struct chart_widget
    {
        struct chart_line
        {
            using container_of_points = small_vector<chart::real_point2d>;

            static constexpr auto background_color = gl::colors::white_f;
            static constexpr auto line_color = gl::colors::red_f;

            stretchable_pxrectangle geometry{};
            container_of_points points{};
            gl::texture2d texture_cache{};
            chart::space_diagonal_cache space_diagonal_cache{};

            bool operator () (const widget::window&) noexcept
            {
                texture_cache = gl::create_texture2d();
                if (!texture_cache)
                {
                    e_debug("chart_line: create texture error: {}", glGetError());
                    return false;
                }

                return true;
            }

            event_result operator () (const ui::size_event&) noexcept
            {
                return event_result::redraw;
            }

            void clear_texture_cache() noexcept
            {
                texture_cache = gl::sizes(std::move(texture_cache), 0_px, 0_px);
            }


            void clear_cache() noexcept
            {
                space_diagonal_cache.clear();
                clear_texture_cache();
            }

            using mouse_move_event_type = widget::mouse_move_event<ui::user_gesture>;

            using redraw_event_type = widget::redraw_event<
                widget::shader::gray_texture_mix_color,
                widget::shader::colored_rectangle,
                buffer_view,
                widget::content_size2d
            >;

            event_result operator () (const ui::mouse_wheel_event& e) noexcept
            {
                if (space_diagonal_cache)
                {
                    const auto diagonal0 = space_diagonal_cache.value();

                    constexpr double zoom_factor = 1.1;
                    const auto zoom = pow(zoom_factor, e.rot());
                    const auto half_d_d_diagonal = (diagonal0._1 - diagonal0._0) * (0.5 * zoom - 0.5);

                    const chart::space_diagonal_t new_diagonal
                    {
                        ._0{ diagonal0._0 - half_d_d_diagonal },
                        ._1{ diagonal0._1 + half_d_d_diagonal }
                    };

                    if (space_diagonal_cache.try_update(new_diagonal))
                    {
                        clear_texture_cache();
                        return event_result::redraw;
                    }
                }

                return event_result::idle;
            }


            widget::event_result operator () (mouse_move_event_type e) noexcept
            {
                if (e.keys().is_left() && has_image(texture_cache) && space_diagonal_cache)
                {
                    const auto texture_sizes = sizes(texture_cache);

                    const auto gesture = e.as_first();
                    const auto has_shift = (zero_v<> != md_trunc_cast<pxoff2d>(gesture.shift()));
                    const auto has_scale = (texture_sizes != gesture.transformation_as(texture_sizes));

                    if (has_shift || has_scale)
                    {
                        const auto diagonal0 = space_diagonal_cache.value();
                        auto new_diagonal = diagonal0;

                        if (has_shift)
                        {
                            const auto scale_to_chart = make_scale_transformation
                            (
                                chart::make_pix_space_diagonal(texture_sizes),
                                diagonal0
                            );

                            const auto chart_shift = -scale_to_chart(gesture.shift());
                            new_diagonal._0 += chart_shift;
                            new_diagonal._1 += chart_shift;
                        }

                        if (has_scale)
                        {
                            const point2d d0
                            {
                                diagonal0._1.x() - diagonal0._0.x(),
                                diagonal0._0.y() - diagonal0._1.y()
                            };

                            const auto d = d0 / gesture.scale();

                            new_diagonal._0.ref_y() = new_diagonal._1.y() + d.y();
                            new_diagonal._1.ref_x() = new_diagonal._0.x() + d.x();
                        }

                        if (space_diagonal_cache.try_update(new_diagonal))
                        {
                            clear_texture_cache();
                            return event_result::redraw;
                        }
                    }
                }

                return event_result::idle;
            }

            void operator () (redraw_event_type e) noexcept
            {
                if (const auto chart_sizes = stretchable_sizes(geometry, e.get<widget::content_size2d>());
                    chart_sizes != sizes(texture_cache))
                {
                    const auto chart_image = px::zeros_pix8space(e.get<buffer_view>(), chart_sizes);

                    if (space_diagonal_cache.try_update(points))
                    {
                        px::draw_polyline(chart_image, points, make_transformation
                        (
                            space_diagonal_cache.value(),
                            chart::make_pix_space_diagonal(chart_sizes)
                        ));
                    }

                    texture_cache = gl::write(std::move(texture_cache), chart_image);
                }

                const pxrectangle view_geometry
                {
                    .position{ geometry.position },
                    .sizes{ sizes(texture_cache) }
                };

                e.get<widget::shader::colored_rectangle>().draw(view_geometry, background_color);
                e.get<widget::shader::gray_texture_mix_color>().draw(view_geometry.position, texture_cache, line_color);
            }

            template<class Fn>
            decltype(auto) apply(Fn fn) noexcept
            {
                return fn
                (
                    widget::ex_context_v<redraw_event_type>,
                    widget::ex_context_v<mouse_move_event_type>
                );
            }
        };

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

        chart_line line
        {
            .geometry{ chart_boundaries }
        };

        chart_line::container_of_points points{};

        bool operator () (const widget::window& w) noexcept
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
                if (!line.points.size())
                {
                    line.points = std::move(points);
                }

                line.clear_cache();
            };

            b_clear.clicked = [this] () noexcept
            {
                if (line.points.size())
                {
                    points = std::move(line.points);
                    line.clear_cache();
                }
            };

            b_exit.clicked = [&w] () noexcept
            {
                ui::quit(w);
            };

            b_plot.clicked();

            return true;
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(b_plot, b_clear, b_exit, line);
        }
    };
}


int app_main(os::module_handle_t app) noexcept
{
    return widget::run<chart_widget>(app);
}