#include "chart_widget.h"

#include <px/algorithm.h>

#include <utility/px.h>

#include <widget/stretchable.h>


namespace chart
{
    event_result chart_widget::operator()(const ui::mouse_wheel_event& e) noexcept
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
                clear_draw_cache();
                return event_result::redraw;
            }
        }

        return event_result::idle;
    }

    event_result chart_widget::operator()(mouse_move_event_type e) noexcept
    {
        if (e.keys().is_left() && space_diagonal_cache)
        {
            if (const auto chart_sizes = stretchable_sizes(geometry, e);
                has_sizes(chart_sizes) && chart_sizes == sizes(line.texture_cache))
            {
                const auto gesture = e.get<ui::user_gesture>();
                const auto has_shift = (zero_v<> != md_trunc_cast<pxoff2d>(gesture.shift()));
                const auto has_scale = (chart_sizes != gesture.transformation_as(chart_sizes));

                if (has_shift || has_scale)
                {
                    const auto diagonal0 = space_diagonal_cache.value();
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

                    if (space_diagonal_cache.try_update(new_diagonal))
                    {
                        clear_draw_cache();
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

        e.get<shader::colored_rectangle>().draw(view_geometry, background_color);

        if (chart_sizes != sizes(line.texture_cache))
        {
            if (space_diagonal_cache.try_update(line.points))
            {
                const auto chart_image = px::zeros_pix8space(e.get<buffer_view>(), chart_sizes);

                px::draw_polyline(chart_image, line.points, make_transformation
                (
                    space_diagonal_cache.value(),
                    make_pix_space_diagonal(chart_sizes)
                ));

                if (line.texture_cache)
                {
                    line.texture_cache = gl::image(std::move(line.texture_cache), chart_image);
                }
                else
                {
                    line.texture_cache = gl::create_texture2d(chart_image);
                }
            }
        }

        e.get<shader::gray_texture_mix_color>().draw
        (
            view_geometry.position,
            line.texture_cache,
            line.pen_color
        );
    }
}
