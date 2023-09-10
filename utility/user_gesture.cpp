#include "user_gesture.h"

#include <ui/event.h>


namespace
{
    constexpr bool gesture_point_is_invalid(gesture_point_value_t value) noexcept
    {
        return invalid_gesture_point_value == value;
    }

    template<class T>
    constexpr bool gesture_point_is_invalid(const vec2<T> v) noexcept
    {
        return gesture_point_is_invalid(v._0)
            || gesture_point_is_invalid(v._1);
    }

    constexpr bool gesture_point_is_vec(const gesture_vpoint2d_t& v) noexcept
    {
        return !gesture_point_is_invalid(v._1._0);
    }

    constexpr bool gesture_point_has_value(const gesture_vpoint2d_t& v) noexcept
    {
        return !gesture_point_is_invalid(v._0._0);
    }

    user_gesture make_motion_user_gesture(const gesture_vpoint2d_t& v0, const gesture_vpoint2d_t& v1) noexcept
    {
        constexpr auto move_zoom_gesture = [] (const gesture_vpoint2d_t& v0, const gesture_vpoint2d_t& v1) noexcept
        {
            constexpr auto center = [] (const gesture_vpoint2d_t& v) noexcept
            {
                return 0.5 * (v._0 + v._1);
            };

            const auto center0 = center(v0);
            const auto center1 = center(v1);
            return center1 - center0;
        };

        constexpr auto zoom_gesture = [] (const gesture_vpoint2d_t& v0, const gesture_vpoint2d_t& v1) noexcept
        {
            constexpr auto distance = [] (const gesture_vpoint2d_t& v) noexcept
            {
                const auto dpt = v._1 - v._0;
                return vabs(dpt);
            };

            const auto distance0 = distance(v0);
            const auto distance1 = distance(v1);
            return distance1 - distance0;
        };

        return user_gesture::instance
        (
            move_zoom_gesture(v0, v1),
            zoom_gesture(v0, v1)
        );
    }

    user_gesture make_motion_user_gesture(const gesture_point2d_t& p0, const gesture_point2d_t& p1) noexcept
    {
        return user_gesture::instance(p1 - p0);
    }

    user_gesture make_motion_user_gesture_opt(const gesture_vpoint2d_t& v0, const gesture_vpoint2d_t& v1) noexcept
    {
        D_ASSERT(!gesture_point_is_invalid(v1));

        if (gesture_point_is_vec(v0))
        {
            D_ASSERT(!gesture_point_is_invalid(v0));

            const auto dv0 = (v0._1 - v0._0);
            const auto dv1 = (v1._1 - v1._0);

            constexpr auto sqrlen = [] <class T> (const vec2<T>&dv) noexcept
            {
                return dv._0 * dv._0 + dv._1 * dv._1;
            };

            const auto [sv0, sv1] = (inner_product(dv0, dv1) >= -numeric_eps_v<gesture_point_value_t>)
                ? vec2{ v0, v1 } : ((sqrlen(dv1) < sqrlen(dv0)) ? vec2{ v0, inverse(v1) } : vec2{ inverse(v0), v1 });

            return make_motion_user_gesture(sv0, sv1);
        }

        return no_user_gesture;
    }

    user_gesture make_motion_user_gesture_opt(const gesture_vpoint2d_t& v, const gesture_point2d_t& p) noexcept
    {
        D_ASSERT(!gesture_point_is_invalid(p));

        if (gesture_point_has_value(v) && !gesture_point_is_vec(v))
        {
            D_ASSERT(!gesture_point_is_invalid(v._0));
            D_ASSERT(invalid_gesture_point == v._1);
            return make_motion_user_gesture(v._0, p);
        }

        return no_user_gesture;
    }

    user_gesture new_motion_user_gesture(gesture_vpoint2d_t& cached_p, gesture_vpoint2d_t new_p) noexcept // constexpr c++23
    {
        return make_motion_user_gesture_opt(std::exchange(cached_p, new_p), new_p);
    }

    user_gesture new_motion_user_gesture(gesture_vpoint2d_t& cached_p, gesture_point2d_t new_p) noexcept // constexpr c++23
    {
        const vec2 new_vp{ new_p, invalid_gesture_point };
        return make_motion_user_gesture_opt(std::exchange(cached_p, new_vp), new_p);
    }

    constexpr gesture_point2d_t to_gesture_point(const ui::pointer_event::point2d_type& p) noexcept
    {
        return
        {
            numeric_cast<gesture_point_value_t>(p.x()),
            numeric_cast<gesture_point_value_t>(p.y())
        };
    }
}


user_gesture new_motion_user_gesture(gesture_vpoint2d_t& cached_p, const ui::pointer_event& e) noexcept
{
    switch (e.size())
    {
        case 1_uz: return new_motion_user_gesture(cached_p, to_gesture_point(e.pointer()));
        case 2_uz:
            return new_motion_user_gesture
            (
                cached_p,
                vec2
                { 
                    to_gesture_point(e.pointer(0_uz)), 
                    to_gesture_point(e.pointer(1_uz)) 
                }
            );

        default:
            break;
    }

    return no_user_gesture;
}