#include "user_gesture.h"

#include <ui/event.h>


namespace
{
    template<class T>
    [[nodiscard]] constexpr bool is_positive_or_near_zero(T value) noexcept
    {
        constexpr auto near_zero_min = -numeric_eps_v<T>;
        return value >= near_zero_min;
    }

    [[nodiscard]]
    constexpr bool cache_has_value(user_motion_cache_value_t value) noexcept
    {
        return no_user_motion_value != value;
    }

    template<class T>
    [[nodiscard]] constexpr bool cache_has_value(const vec2<T>& v) noexcept
    {
        return cache_has_value(v._0) && cache_has_value(v._1);
    }

    [[nodiscard]]
    constexpr size_t size(const user_motion_cache_t& v) noexcept
    {
        size_t result{ 0u };

        if (cache_has_value(v._0._0))
        {
            ++result;
            if (cache_has_value(v._1._0))
            {
                ++result;
            }
        }

        return result;
    }

    [[nodiscard]] 
    constexpr bool cache_is_valid(const user_motion_cache_t& v) noexcept
    {
        switch (size(v))
        {
            case 0u: return (no_user_motion == v);
            case 1u: return (cache_has_value(v._0) && (no_user_motion_point == v._1));
            case 2u: return cache_has_value(v);

            default:
                break;
        }

        return false;
    }

    [[nodiscard]]
    user_gesture make_gesture(const vec2<user_motion_cache_t>& v) noexcept
    {
        constexpr auto move_zoom_gesture = [] (const vec2<user_motion_cache_t>& v) noexcept
        {
            constexpr auto center = [] (const user_motion_cache_t& v) noexcept
            {
                return 0.5 * (v._0 + v._1);
            };

            const auto center0 = center(v._0);
            const auto center1 = center(v._1);
            return center1 - center0;
        };

        constexpr auto zoom_gesture = [] (const vec2<user_motion_cache_t>& v) noexcept
        {
            constexpr auto distance = [] (const user_motion_cache_t& v) noexcept
            {
                const auto dpt = v._1 - v._0;
                return vabs(dpt);
            };

            const auto distance0 = distance(v._0);
            const auto distance1 = distance(v._1);
            return distance1 - distance0;
        };

        return
        {
            .move{ vtrunc_to_pxz(move_zoom_gesture(v)) },
            .zoom{ vtrunc_to_pxz(zoom_gesture(v)) }
        };
    }

    [[nodiscard]]
    constexpr vec2<user_motion_cache_t> codirectional(const user_motion_cache_t& v0, const user_motion_cache_t& v1) noexcept
    {
        const auto dv0 = (v0._1 - v0._0);
        const auto dv1 = (v1._1 - v1._0);

        constexpr auto sqrlen = [] <class T> (const vec2<T>&dv) noexcept
        {
            return dv._0 * dv._0 + dv._1 * dv._1;
        };

        return is_positive_or_near_zero(inner_product(dv0, dv1))
            ? vec2{ v0, v1 } : ((sqrlen(dv1) < sqrlen(dv0)) ? vec2{ v0, inverse(v1) } : vec2{ inverse(v0), v1 });
    }

    [[nodiscard]]
    user_gesture make_gesture(const user_motion_cache_point2d_t& p0, const user_motion_cache_point2d_t& p1) noexcept
    {
        return
        {
            .move{ vtrunc_to_pxz(p1 - p0) },
            .zoom{ no_gesture_pxoff2d }
        };
    }

    [[nodiscard]]
    user_gesture make_gesture_opt(const user_motion_cache_t& v0, const user_motion_cache_t& v1) noexcept
    {
        return (2u == size(v0)) ? make_gesture(codirectional(v0, v1)) : no_gesture;
    }

    [[nodiscard]]
    user_gesture make_gesture_opt(const user_motion_cache_t& v, const user_motion_cache_point2d_t& p) noexcept
    {
        return (1u == size(v)) ? make_gesture(v._0, p) : no_gesture;
    }

    [[nodiscard]]
    user_gesture new_user_motion(user_motion_cache_t& cached_p, user_motion_cache_t  new_p) noexcept // constexpr c++23
    {
        return make_gesture_opt(std::exchange(cached_p, new_p), new_p);
    }

    [[nodiscard]]
    user_gesture new_user_motion(user_motion_cache_t& cached_p, user_motion_cache_point2d_t new_p) noexcept // constexpr c++23
    {
        return make_gesture_opt
        (
            std::exchange(cached_p, vec2{ new_p, no_user_motion_point }),
            new_p
        );
    }

    [[nodiscard]]
    constexpr user_motion_cache_point2d_t to_cached_point(const ui::pointer_event::point2d_type& p) noexcept
    {
        return
        {
            numeric_cast<user_motion_cache_value_t>(p.x()),
            numeric_cast<user_motion_cache_value_t>(p.y())
        };
    }
}


user_gesture new_user_motion(user_motion_cache_t& cached_p, const ui::pointer_event& e) noexcept
{
    D_ASSERT(cache_is_valid(cached_p));

    switch (e.size())
    {
        case 1_uz: return new_user_motion(cached_p, to_cached_point(e.pointer()));
        case 2_uz:
            return new_user_motion
            (
                cached_p,
                vec2
                {
                    to_cached_point(e.pointer(0_uz)),
                    to_cached_point(e.pointer(1_uz))
                }
            );

        default:
            break;
    }

    cached_p = no_user_motion;
    return no_gesture;
}
