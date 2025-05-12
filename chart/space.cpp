#include "space.h"


namespace chart
{
    event_result space::process(const mouse_wheel_event<>& e)  noexcept
    {
        if (has_space(e))
        {
            constexpr double zoom_factor = 1.1;
            const auto nzoom = pow(zoom_factor, e.rot());

            const auto diagonal = space_cache.value();
            const auto half_d_d_diagonal = (diagonal._1 - diagonal._0) * (0.5 * nzoom - 0.5);

            const space_diagonal new_diagonal
            {
                ._0{ diagonal._0 - half_d_d_diagonal },
                ._1{ diagonal._1 + half_d_d_diagonal }
            };

            sizes_cache = px::no_sizes;
            D_UNUSED(space_cache.try_update(new_diagonal, geometry_cache.sizes));
            return event_result::redraw;
        }

        return event_result::idle;
    }

    event_result space::process(const gesture_event<>& e) noexcept
    {
        if (has_space(e))
        {
            const auto has_shift = (zero_v<> != md_trunc_cast<pxoffs>(e.shift()));
            const auto has_scale = (geometry_cache.sizes != e.transformation_as(geometry_cache.sizes));

            if (has_shift || has_scale)
            {
                const auto diagonal0 = space_cache.value();
                auto new_diagonal = diagonal0;

                if (has_shift)
                {
                    const auto scale_to_chart = make_scale_transformation
                    (
                        make_pxspace_diagonal(geometry_cache.sizes),
                        diagonal0
                    );

                    const auto chart_shift = -scale_to_chart(e.shift());
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

                    const auto d = d0 / e.scale();

                    new_diagonal._0.ref_y() = new_diagonal._1.y() + d.y();
                    new_diagonal._1.ref_x() = new_diagonal._0.x() + d.x();
                }

                sizes_cache = px::no_sizes;
                D_UNUSED(space_cache.try_update(new_diagonal, geometry_cache.sizes));
                return event_result::redraw;
            }
        }

        return event_result::idle;
    }
}
