#include <widget/run.h>


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
        bool operator () (viewport_size2d viewport) noexcept
        {
            texture_ = pix8map_test_texture_generate(viewport / 4u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(viewport, texture_))
            {
                return false;
            }

            area_ = default_area(viewport);
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

        event_result operator () (widget::mouse_double_click_event<viewport_size2d> e) noexcept
        {
            area_ = default_area(e.as_first());
            return event_result::redraw;
        }

        using mouse_move_event_type = widget::mouse_move_event<widget::user_gesture>;

        event_result operator () (mouse_move_event_type e) noexcept
        {
            
            if(const auto new_area =  e.as_first().transformation_as(area_); 
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
        decltype(auto) apply(Fn fn) const noexcept
        {
            return fn(widget::ex_context_v<mouse_move_event_type>);
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




