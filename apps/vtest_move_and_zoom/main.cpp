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
        bool initialize(os::module_handle_t app) noexcept
        {
            egl_ = create_egl_ui(app);
            if (!egl_)
            {
                return false;
            }

            return reinitialize();
        }

        [[nodiscard]]
        bool reinitialize() noexcept
        {
            texture_ = pix8map_test_texture_generate(egl_.viewport / 4u);
            if (!texture_)
            {
                return false;
            }

            if (!shaders_.initialize(egl_.viewport, texture_))
            {
                return false;
            }

            area_ = default_area(egl_.viewport);

            return true;
        }

        std::nullopt_t operator () (const ui::mouse_wheel_event& e) noexcept
        {
            if (const auto new_area = zoom_increase(area_, e.rot()); new_area != area_)
            {
                area_ = new_area;
                need_redraw_ = true;
            }

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_double_click_event&) noexcept
        {
            area_ = default_area(egl_.viewport);
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

            if(const auto new_area = new_manipulation(user_vpoint_cache_, e).transformation_as(area_); 
                new_area != area_ && is_safe_conversion_glpx(new_area))
            {
                area_ = new_area;
                need_redraw_ = true;
            }

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_up_event&) noexcept
        {
            user_vpoint_cache_ = ui::no_cached_user_vpoint;
            return std::nullopt;
        }

        void operator () (ui::content_rect_changed_event)
        {
            const auto new_viewport = app_ui_viewport_request(egl_);
            if (!new_viewport)
            {
                ui_fatal_debug(egl_, "content rect error: ui error: {}, egl error: {}",
                    ui::error_code(), eglGetError());
                return;
            }

            if (new_viewport != egl_.viewport)
            {
                egl_.viewport = new_viewport;

                if (!reinitialize())
                {
                    ui_fatal_debug(egl_, "reinitialize viewport error: ui error: {}, egl error: {}",
                        ui::error_code(), eglGetError());
                    return;
                }
            }

            return;
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
            gl::viewport(egl_.viewport);
            gl::clear(gl::colors::white_f);
            shaders_.draw(area_);
        }

    private:
        egl_ui_owner egl_{};
        shaders_lib shaders_{};
        gl::texture2d texture_{};
        ui::user_vpoint_cache user_vpoint_cache_{ ui::no_cached_user_vpoint };
        pxzrectangle area_{};
        bool need_redraw_{ true };
    };
}

int app_main(os::module_handle_t app) noexcept
{
    main_processor processor{};

    if (!processor.initialize(app))
    {
        e_debug("create window error: ui error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return processor.run();
}




