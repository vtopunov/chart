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
            texture_ = make_test_lumpixmap_texture(e.viewport() / 4u);
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
            return widget::redraw_if(apply_nzoom(area_, e.rot()));
        }

        constexpr event_result operator () (widget::mouse_double_click_event<> e) noexcept
        {
            area_ = default_area(e.viewport());
            return event_result::redraw;
        }

        using gesture_event_type = widget::basic_gesture_event<>;

        constexpr event_result operator () (gesture_event_type e) noexcept
        {
            return widget::redraw_if(px::update_pxf(area_, e.transformation_as(area_)));
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
        figure_area area_{};
    };
}

int main() noexcept
{
    return widget::run<main_widget>();
}




