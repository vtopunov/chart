#pragma once

#include <utility/user_gesture.h>

#include <ui/event.h>

#include <widget/fwd.h>


namespace widget
{
    class gesture_cache
    {
    public:
        constexpr gesture_cache() noexcept = default;
        D_DISABLE_COPY_MOVE(gesture_cache);

        constexpr bool has_move() const noexcept
        {
            return gesture_cahce_.has_move();
        }

        constexpr bool has_zoom() const noexcept
        {
            return gesture_cahce_.has_zoom();
        }

        constexpr bool has_gesture() const noexcept
        {
            return gesture_cahce_.has_gesture();
        }

        constexpr user_gesture_pxoff2d zoom() const noexcept
        {
            return gesture_cahce_.zoom;
        }

        constexpr user_gesture_pxoff2d move() const noexcept
        {
            return gesture_cahce_.move;
        }

        constexpr explicit operator bool () const noexcept
        {
            return !!gesture_cahce_;
        }

        void operator () (const ui::mouse_up_event&) noexcept
        {
            clear();
        }

        void operator () (const ui::mouse_move_event& e) noexcept
        {
#if  defined(D_OS_WINDOWS)
            if (!e.keys().is_left())
            {
                clear();
                return;
            }
#endif

            gesture_cahce_ =  new_user_motion(position_cache_, e);
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }

    private:
        void clear() noexcept
        {
            position_cache_ = no_user_motion;
            gesture_cahce_ = no_gesture;
        }

    private:
        ::user_motion_cache_t position_cache_{ no_user_motion };
        ::user_gesture gesture_cahce_{ no_gesture };
    };
}
