#include "user_gesture.h"

#include <ui/event.h>


namespace
{
    using gesture_cache_value_t = user_gesture_cache::value_type;
    using gesture_cache_point2d_t = user_gesture_cache::point2d_type;
    using gesture_cache_vpoint2d_t = user_gesture_cache::vpoint2d_type;

    constexpr user_gesture no_gesture
    {
        .move{ user_gesture::no_px },
        .zoom{ user_gesture::no_px }
    };

    template<class T>
    constexpr bool is_positive_or_near_zero(T value) noexcept
    {
        constexpr auto near_zero_min = -numeric_eps_v<T>;
        return value >= near_zero_min;
    }

    constexpr bool gesture_cache_is_invalid(gesture_cache_value_t value) noexcept
    {
        return user_gesture_cache::invalid_value == value;
    }

    template<class T>
    constexpr bool gesture_cache_is_invalid(const vec2<T> v) noexcept
    {
        return gesture_cache_is_invalid(v._0)
            || gesture_cache_is_invalid(v._1);
    }

    constexpr bool gesture_point_is_vec(const gesture_cache_vpoint2d_t& v) noexcept
    {
        return !gesture_cache_is_invalid(v._1._0);
    }

    constexpr bool gesture_point_has_value(const gesture_cache_vpoint2d_t& v) noexcept
    {
        return !gesture_cache_is_invalid(v._0._0);
    }

    user_gesture make_motion_gesture(const gesture_cache_vpoint2d_t& v0, const gesture_cache_vpoint2d_t& v1) noexcept
    {
        constexpr auto move_zoom_gesture = [] (const gesture_cache_vpoint2d_t& v0, const gesture_cache_vpoint2d_t& v1) noexcept
        {
            constexpr auto center = [] (const gesture_cache_vpoint2d_t& v) noexcept
            {
                return 0.5 * (v._0 + v._1);
            };

            const auto center0 = center(v0);
            const auto center1 = center(v1);
            return center1 - center0;
        };

        constexpr auto zoom_gesture = [] (const gesture_cache_vpoint2d_t& v0, const gesture_cache_vpoint2d_t& v1) noexcept
        {
            constexpr auto distance = [] (const gesture_cache_vpoint2d_t& v) noexcept
            {
                const auto dpt = v._1 - v._0;
                return vabs(dpt);
            };

            const auto distance0 = distance(v0);
            const auto distance1 = distance(v1);
            return distance1 - distance0;
        };

        return
        {
            .move{ vtrunc_to_pxz(move_zoom_gesture(v0, v1)) },
            .zoom{ vtrunc_to_pxz(zoom_gesture(v0, v1)) }
        };
    }

    user_gesture make_motion_gesture(const gesture_cache_point2d_t& p0, const gesture_cache_point2d_t& p1) noexcept
    {
        return
        {
            .move{ vtrunc_to_pxz(p1 - p0) },
            .zoom{ user_gesture::no_px }
        };
    }

    user_gesture make_motion_gesture_opt(const gesture_cache_vpoint2d_t& v0, const gesture_cache_vpoint2d_t& v1) noexcept
    {
        D_ASSERT(!gesture_cache_is_invalid(v1));

        if (gesture_point_is_vec(v0))
        {
            D_ASSERT(!gesture_cache_is_invalid(v0));

            const auto dv0 = (v0._1 - v0._0);
            const auto dv1 = (v1._1 - v1._0);

            constexpr auto sqrlen = [] <class T> (const vec2<T>&dv) noexcept
            {
                return dv._0 * dv._0 + dv._1 * dv._1;
            };

            const auto [sv0, sv1] = is_positive_or_near_zero(inner_product(dv0, dv1))
                ? vec2{ v0, v1 } : ((sqrlen(dv1) < sqrlen(dv0)) ? vec2{ v0, inverse(v1) } : vec2{ inverse(v0), v1 });

            return make_motion_gesture(sv0, sv1);
        }

        return no_gesture;
    }

    user_gesture make_motion_gesture_opt(const gesture_cache_vpoint2d_t& v, const gesture_cache_point2d_t& p) noexcept
    {
        D_ASSERT(!gesture_cache_is_invalid(p));

        if (gesture_point_has_value(v) && !gesture_point_is_vec(v))
        {
            D_ASSERT(!gesture_cache_is_invalid(v._0));
            D_ASSERT(user_gesture_cache::invalid_point == v._1);
            return make_motion_gesture(v._0, p);
        }

        return no_gesture;
    }

    user_gesture new_motion_gesture(gesture_cache_vpoint2d_t& cached_p, gesture_cache_vpoint2d_t new_p) noexcept // constexpr c++23
    {
        return make_motion_gesture_opt(std::exchange(cached_p, new_p), new_p);
    }

    user_gesture new_motion_gesture(gesture_cache_vpoint2d_t& cached_p, gesture_cache_point2d_t new_p) noexcept // constexpr c++23
    {
        return make_motion_gesture_opt
        (
            std::exchange(cached_p, vec2{ new_p, user_gesture_cache::invalid_point }),
            new_p
        );
    }

    constexpr gesture_cache_point2d_t to_gesture_point(const ui::pointer_event::point2d_type& p) noexcept
    {
        return
        {
            numeric_cast<gesture_cache_value_t>(p.x()),
            numeric_cast<gesture_cache_value_t>(p.y())
        };
    }
}

user_gesture user_gesture_cache::new_motion_gesture(const ui::pointer_event& e) noexcept
{
    switch (e.size())
    {
        case 1_uz: return ::new_motion_gesture(vpoint_, to_gesture_point(e.pointer()));
        case 2_uz:
            return ::new_motion_gesture
            (
                vpoint_,
                vec2
                {
                    to_gesture_point(e.pointer(0_uz)),
                    to_gesture_point(e.pointer(1_uz))
                }
            );

        default:
            break;
    }

    return no_gesture;
}
