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

struct user_gesture
{
    static constexpr pxoff2d no_px{ 0_pxz, 0_pxz };

    pxoff2d move;
    pxoff2d zoom;

    [[nodiscard]]
    constexpr bool has_move() const noexcept
    {
        return no_px != move;
    }

    [[nodiscard]]
    constexpr bool has_zoom() const noexcept
    {
        return no_px != zoom;
    }
};

class user_gesture_cache
{
public:
    using value_type = double;
    using point2d_type = point2d<value_type>;
    using vpoint2d_type = vec2<point2d_type>;

    static constexpr auto invalid_value = numeric_max_v<value_type>;
    static constexpr auto invalid_point = fill_to<point2d>(invalid_value);
    static constexpr auto invalid_vpoint = fill_vec2(invalid_point);

    user_gesture new_motion_gesture(const ui::pointer_event& e) noexcept;

    constexpr void clear() noexcept
    {
        vpoint_ = invalid_vpoint;
    }

private:
    vpoint2d_type vpoint_{ invalid_vpoint };
};
