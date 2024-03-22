#include <widget/run.h>
#include <widget/ex_context.h>


#include "vtest_move_and_zoom_fwd.h"

namespace
{
#include "vtest_move_and_zoom_impl.h"

    using namespace vtest_move_and_zoom;
}

using widget::event_result;


namespace
{
    class main_widget
    {
    public:
        bool operator () (widget::viewport_event<> e) noexcept
        {
            texture_ = pix8map_test_texture_generate(e.viewport() / 4u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(e.viewport(), texture_))
            {
                return false;
            }

            area_ = default_area(e.viewport());
            return true;
        }

        event_result operator () (const ui::mouse_wheel_event& e) noexcept
        {
            if (const auto new_area = zoom_increase(area_, e.rot()); new_area != area_)
            {
                area_ = new_area;
                return event_result::redraw;
            }

            return event_result::idle;
        }

        event_result operator () (widget::mouse_double_click_event<> e) noexcept
        {
            area_ = default_area(e.viewport());
            return event_result::redraw;
        }

        using gesture_event_type = widget::gesture_event<>;

        event_result operator () (gesture_event_type e) noexcept
        {
            if(const auto new_area =  e.get<ui::gesture>().transformation_as(area_); 
                new_area != area_ && is_safe_conversion_glpx(new_area))
            {
                area_ = new_area;
                return event_result::redraw;
            }

            return event_result::idle;
        }

        void operator () (widget::redraw_event<>) const noexcept
        {
            shaders_.draw(area_);
        }

        template<class Fn>
        constexpr decltype(auto) apply(Fn&& fn) const noexcept
        {
            return widget::ex_context_v<gesture_event_type>(std::forward<Fn>(fn));
        }

    private:
        shaders_lib shaders_{};
        gl::texture2d texture_{};
        pxzrectangle area_{};
    };
}

int app_main(os::module_handle_t app) noexcept
{
    return widget::run<main_widget>(app);
}




