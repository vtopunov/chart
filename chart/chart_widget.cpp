#include "chart_widget.h"

#include <px/algorithm.h>

#include <utility/px.h>

#include <widget/stretchable.h>


namespace chart
{
    namespace
    {
        [[nodiscard]]
        constexpr bool try_update_lines_space(space_diagonal_cache& cache, intrusive_list_view<const chart_line> lines) noexcept
        {
            if (cache.has_value())
            {
                return true;
            }

            space_diagonal_t diagonal{ space_diagonal_initializer };
            for (const auto& line : lines)
            {
                diagonal = space_diagonal_with(diagonal, line.points);
            }

            return cache.try_update(diagonal);
        }

        template<class Transformation>
        void draw_chart_lines(pix8span pixs, intrusive_list_view<chart_line> lines, const Transformation value2px) noexcept
        {
            for (auto& line : lines)
            {
                zero_memory(pixs);
                px::draw_polyline(pixs, line.points, value2px);

                if (line.texture_cache)
                {
                    gl::write(line.texture_cache, pixs);
                }
                else
                {
                    line.texture_cache =
                    {
                        resource_construct,
                        gl::create_texture2d(pixs).release()
                    };
                }

                D_ASSERT(line.texture_cache);
            }
        }
    }

    event_result chart_widget::operator()(const ui::mouse_wheel_event& e) noexcept
    {
        if (lines_space_cache)
        {
            const auto diagonal0 = lines_space_cache.value();

            constexpr double zoom_factor = 1.1;
            const auto zoom = pow(zoom_factor, e.rot());
            const auto half_d_d_diagonal = (diagonal0._1 - diagonal0._0) * (0.5 * zoom - 0.5);

            const chart::space_diagonal_t new_diagonal
            {
                ._0{ diagonal0._0 - half_d_d_diagonal },
                ._1{ diagonal0._1 + half_d_d_diagonal }
            };

            if (lines_space_cache.try_update(new_diagonal))
            {
                chart_space_cache = {};
                return event_result::redraw;
            }
        }

        return event_result::idle;
    }

    event_result chart_widget::operator()(mouse_move_event_type e) noexcept
    {
        if (e.keys().is_left() && lines_space_cache)
        {
            if (const auto chart_sizes = stretchable_sizes(geometry, e);
               chart_sizes.width() && chart_sizes.height() && (chart_sizes == chart_space_cache))
            {
                const auto gesture = e.get<ui::user_gesture>();
                const auto has_shift = (zero_v<> != md_trunc_cast<pxoff2d>(gesture.shift()));
                const auto has_scale = (chart_sizes != gesture.transformation_as(chart_sizes));

                if (has_shift || has_scale)
                {
                    const auto diagonal0 = lines_space_cache.value();
                    auto new_diagonal = diagonal0;

                    if (has_shift)
                    {
                        const auto scale_to_chart = make_scale_transformation
                        (
                            make_pix_space_diagonal(chart_sizes),
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

                    if (lines_space_cache.try_update(new_diagonal))
                    {
                        chart_space_cache = {};
                        return event_result::redraw;
                    }
                }
            }
        }

        return event_result::idle;
    }

    void chart_widget::operator()(redraw_event_type e) noexcept
    {
        const auto chart_sizes = stretchable_sizes(geometry, e);

        const pxrectangle view_geometry
        {
            .position{ geometry.position },
            .sizes{ chart_sizes }
        };

        e.get<shader::colored_rectangle>()
            .use()
            .store(view_geometry)
            .store(background_color)
            .draw();

        if (chart_sizes != chart_space_cache)
        {
            chart_space_cache = chart_sizes;

            if (try_update_lines_space(lines_space_cache, lines))
            {
                const auto image = px::create_pix8span(e.get<buffer_view>(), chart_sizes);
                draw_chart_lines(image, lines, make_transformation
                (
                    lines_space_cache.value(),
                    make_pix_space_diagonal(chart_sizes)
                ));
            }
        }

        {
            const auto& shdr = e.get<shader::gray_texture_mix_color>()
                .use()
                .store(view_geometry);

            for (const auto& line : std::as_const(lines))
            {
                shdr.store(line.pen_color)
                    .store(line.texture_cache)
                    .draw();
            }
        }
    }
}
