#pragma once

#include <ui/manipulator.h>
#include <ui/event.h>


namespace widget
{
    class gesture : public ui::gesture
    {
        using base_type = ui::gesture;

    public:
        constexpr gesture() noexcept
            : base_type{ ui::no_gesture }
        {}

        D_DISABLE_COPYMOVE_CA(gesture);

        void operator () (ui::viewport_event) noexcept
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

        constexpr noapply_t apply(no_overload) const noexcept
        {
            return noapply;
        }

    private:
        void clear() noexcept
        {
            vpoint_cache_ = ui::no_cached_user_vpoint;
            base_ref() = ui::no_gesture;
        }

        constexpr ui::gesture& base_ref() noexcept
        {
            return *this;
        }

    private:
        ui::user_vpoint_cache vpoint_cache_{ ui::no_cached_user_vpoint };
    };
}
