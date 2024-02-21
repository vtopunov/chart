#include "space.h"


namespace chart
{
    event_result space::process(mouse_wheel_event e) noexcept
    {
        if (items_space_cache)
        {
            if (const auto chart_sizes = stretchable_sizes(geometry, e);
                chart_sizes.width() && chart_sizes.height() && (chart_sizes == space_cache))
            {
                const auto n_wheel = e.rot();
                constexpr double zoom_factor = 1.1;
                const auto zoom = pow(zoom_factor, n_wheel);

                const auto diagonal = items_space_cache.value();
                const auto half_d_d_diagonal = (diagonal._1 - diagonal._0) * (0.5 * zoom - 0.5);

                const space_diagonal_t new_diagonal
                {
                    ._0{ diagonal._0 - half_d_d_diagonal },
                    ._1{ diagonal._1 + half_d_d_diagonal }
                };

                if (items_space_cache.try_update(new_diagonal, chart_sizes))
                {
                    space_cache = {};
                    return event_result::redraw;
                }
            }
        }

        return event_result::idle;
    }

    event_result space::process(gesture_event e) noexcept
    {
        if (e.keys().is_left() && items_space_cache)
        {
            if (const auto chart_sizes = stretchable_sizes(geometry, e);
               chart_sizes.width() && chart_sizes.height() && (chart_sizes == space_cache))
            {
                const auto gesture = e.get<ui::gesture>();
                const auto has_shift = (zero_v<> != md_trunc_cast<pxoff2d>(gesture.shift()));
                const auto has_scale = (chart_sizes != gesture.transformation_as(chart_sizes));

                if (has_shift || has_scale)
                {
                    const auto diagonal0 = items_space_cache.value();
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

                    if (items_space_cache.try_update(new_diagonal, chart_sizes))
                    {
                        space_cache = {};
                        return event_result::redraw;
                    }
                }
            }
        }

        return event_result::idle;
    }
}
