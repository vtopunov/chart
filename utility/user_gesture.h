#pragma once

#include <core/round.h>

#include <ui/fwd.h>


template<class T>
[[nodiscard]] pxoff_t trunc_to_pxz(const T& value) noexcept
{
    return trunc_cast<pxoff_t>(value);
}

template<class T>
[[nodiscard]] pxoff2d vtrunc_to_pxz(const vec2<T>& p) noexcept
{
    return
    {
        trunc_to_pxz(p._0),
        trunc_to_pxz(p._1)
    };
}

struct user_gesture_pxoff2d : pxoff2d
{
    static constexpr pxoff2d no_px{ 0_pxz, 0_pxz };

    [[nodiscard]]
    constexpr bool has_offset() const noexcept
    {
        return no_px != *this;
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_offset();
    }
};

constexpr user_gesture_pxoff2d no_gesture_pxoff2d{ user_gesture_pxoff2d::no_px };

struct user_gesture
{
    user_gesture_pxoff2d move;
    user_gesture_pxoff2d zoom;

    [[nodiscard]]
    constexpr bool has_move() const noexcept
    {
        return move.has_offset();
    }

    [[nodiscard]]
    constexpr bool has_zoom() const noexcept
    {
        return zoom.has_offset();
    }

    [[nodiscard]]
    constexpr bool has_gesture() const noexcept
    {
        return has_move() || has_zoom();
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_gesture();
    }
};

using user_motion_cache_value_t = double;
using user_motion_cache_point2d_t = point2d<user_motion_cache_value_t>;;
using user_motion_cache_t = vec2<user_motion_cache_point2d_t>;

constexpr auto no_user_motion_value = numeric_max_v<user_motion_cache_value_t>;
constexpr auto no_user_motion_point = fill_to<point2d>(no_user_motion_value);
constexpr auto no_user_motion = fill_vec2(no_user_motion_point);

[[nodiscard]]
user_gesture new_user_motion(user_motion_cache_t& cached_p, const ui::pointer_event& e) noexcept;

constexpr user_gesture no_gesture
{
    .move{ no_gesture_pxoff2d },
    .zoom{ no_gesture_pxoff2d }
};
