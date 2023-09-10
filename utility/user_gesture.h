#pragma once

#include <core/round.h>

#include <ui/fwd.h>


template<class T>
[[nodiscard]] pxoff2d vtrunc_to_px(const vec2<T>& p) noexcept
{
    return
    {
        trunc_cast<pxoff_t>(p._0),
        trunc_cast<pxoff_t>(p._1)
    };
}

struct user_gesture
{
    static constexpr pxoff2d no_pxoff{ 0_pxz, 0_pxz };

    pxoff2d move;
    pxoff2d zoom;

    template<class M, class Z>
    [[nodiscard]] static user_gesture instance(const point2d<M>& move, const point2d<Z>& zoom) noexcept
    {
        return
        {
            .move{ vtrunc_to_px(move) },
            .zoom{ vtrunc_to_px(zoom) }
        };
    }

    template<class T>
    [[nodiscard]] static user_gesture instance(const point2d<T>& move) noexcept
    {
        return { .move{ vtrunc_to_px(move) }, .zoom{ no_pxoff } };
    }

    [[nodiscard]]
    constexpr bool has_move() const noexcept
    {
        return has_offset(move);
    }

    [[nodiscard]]
    constexpr bool has_zoom() const noexcept
    {
        return has_offset(zoom);
    }

    [[nodiscard]]
    static constexpr bool has_offset(pxoff2d offset) noexcept
    {
        return offset.x() || offset.y();
    }
};

constexpr user_gesture no_user_gesture
{
    .move{ user_gesture::no_pxoff },
    .zoom{ user_gesture::no_pxoff }
};

using gesture_point_value_t = double;
using gesture_point2d_t = point2d<gesture_point_value_t>;
using gesture_vpoint2d_t = vec2<gesture_point2d_t>;

constexpr auto invalid_gesture_point_value = numeric_max_v<gesture_point_value_t>;
constexpr auto invalid_gesture_point = fill_to<point2d>(invalid_gesture_point_value);
constexpr auto invalid_gesture_vpoint = fill_vec2(invalid_gesture_point);

user_gesture new_motion_user_gesture(gesture_vpoint2d_t& cached_p, const ui::pointer_event& e) noexcept;

constexpr void clear_gesture_cache(gesture_vpoint2d_t& cache) noexcept
{
    cache = invalid_gesture_vpoint;
}
