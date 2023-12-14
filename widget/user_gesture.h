#pragma once

#include <ui/manipulator.h>
#include <ui/event.h>

#include <utility/shader_library.h>


namespace widget
{
    class user_gesture : public ui::user_gesture
    {
        using base_type = ui::user_gesture;

    public:
        constexpr user_gesture() noexcept 
            : base_type{ ui::no_gesture } 
        {}

        D_DISABLE_COPY_MOVE(user_gesture);

        void operator () (viewport_size2d) noexcept
        {
            clear();
        }

        void operator () (const ui::mouse_down_event&) noexcept
        {
            clear();
        }

        void operator () (const ui::mouse_up_event&) noexcept
        {
            clear();
        }

        void operator () (const ui::mouse_move_event& e) noexcept
        {
#ifdef D_OS_WINDOWS
            if (!e.keys().is_left())
            {
                clear();
                return;
            }
#endif

            base_ref() = new_manipulation(vpoint_cache_, e);
        }


#ifdef D_OS_WINDOWS
        void operator () (const ui::mouse_wheel_event&) noexcept
        {
            clear();
        }

        void operator () (const ui::mouse_double_click_event&) noexcept
        {
            clear();
        }

#endif

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }

    private:
        void clear() noexcept
        {
            vpoint_cache_ = ui::no_cached_user_vpoint;
            base_ref() = ui::no_gesture;
        }

        constexpr ui::user_gesture& base_ref() noexcept
        {
            return *this;
        }

    private:
        ui::user_vpoint_cache vpoint_cache_{ ui::no_cached_user_vpoint };
    };
}
