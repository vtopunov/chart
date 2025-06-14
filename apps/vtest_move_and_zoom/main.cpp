#include <ui/manipulator.h>

#include "vtest_move_and_zoom_fwd.h"

namespace
{
#include "vtest_move_and_zoom_impl.h"

    using namespace vtest_move_and_zoom;
}


namespace
{
    class main_processor
    {
    public:
        [[nodiscard]]
        bool initialize() noexcept
        {
            egl_ = egli_builder{}.build();
            if (!egl_)
            {
                return false;
            }

            return reinitialize();
        }

        [[nodiscard]]
        bool reinitialize() noexcept
        {
            texture_ = make_test_lumpixmap_texture(egl_.viewport() / 4u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(egl_.viewport(), texture_))
            {
                return false;
            }

            area_ = default_area(egl_.viewport());

            return true;
        }

        std::nullopt_t operator () (const ui::mouse_wheel_event& e) noexcept
        {
            need_redraw_ = apply_nzoom(area_, e.rot());
            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_double_click_event&) noexcept
        {
            area_ = default_area(egl_.viewport());
            need_redraw_ = true;
            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_move_event& e) noexcept
        {
            if (!e.keys().is_left())
            {
                user_vpoint_cache_ = ui::no_cached_user_vpoint;
                return std::nullopt;
            }

            need_redraw_ = px::update_pxf
            (
                area_, 
                new_manipulation(user_vpoint_cache_, e).transformation_as(area_)
            );

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_up_event&) noexcept
        {
            user_vpoint_cache_ = ui::no_cached_user_vpoint;
            return std::nullopt;
        }

        void operator () (ui::content_rect_changed_event)
        {
            D_UNUSED(egl_.update_viewport());
        }

        void operator () (ui::redraw_needed_event) noexcept
        {
            need_redraw_ = true;
        }

        ui::milliseconds operator () (ui::idle_event) noexcept
        {
            if (need_redraw_)
            {
                need_redraw_ = false;
                draw();
            }

            return ui::infinite;
        }

        int run()
        {
            draw();
            return ui::run_event_loop(egl_, *this);
        }

    private:
        void draw() const noexcept
        {
            const egl_painting_owner painting_owner{ egl_ };
            gl::viewport(egl_.viewport());
            gl::clear(colors::white_f);
            shaders_.draw(area_);
        }

    private:
        egli_owner egl_{};
        shaders_lib shaders_{};
        gl::texture2d texture_{};
        ui::user_vpoint_cache user_vpoint_cache_{ ui::no_cached_user_vpoint };
        figure_area area_{};
        bool need_redraw_{ true };
    };
}

int main() noexcept
{
    main_processor processor{};
    if (!processor.initialize())
    {
        e_debug("create window error: {}", egli::error_code());
        return EXIT_FAILURE;
    }

    return processor.run();
}




