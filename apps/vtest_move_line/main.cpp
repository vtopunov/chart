#include <core/round.h>

#include <debug/debug.h>

#include <px/algorithm.h>

#include <egl_ui/event_loop.h>

#include <utility/shaders_library.h>


namespace
{
    gl::texture2d lines_rendering(const pix8span image, const pxoff2d d) noexcept
    {
        const auto draw_line = [image, d] (double x0, double y0, double x1, double y1) noexcept
        {
            draw_antialiasing_line(image, x0 + d.x(), y0 + d.y(), x1 + d.x(), y1 + d.y());
        };

        draw_line(240, 315, 5, 325); // 180+
        draw_line(240, 320, 5, 330);
        draw_line(5, 335, 240, 325);

        draw_line(240, 295, 5, 295); // 180
        draw_line(240, 300, 5, 300);
        draw_line(5, 305, 240, 305);

        draw_line(240, 275, 5, 265); // 180-
        draw_line(240, 280, 5, 270);
        draw_line(5, 275, 240, 285);

        draw_line(240, 255, 5, 40);  // 135+
        draw_line(240, 260, 5, 45);
        draw_line(5, 50, 240, 265);

        draw_line(245, 240, 10, 5);  // 135
        draw_line(240, 240, 5, 5);
        draw_line(5, 10, 240, 245);

        draw_line(265, 240, 50, 5);  // 135-
        draw_line(260, 240, 45, 5);
        draw_line(40, 5, 255, 240);

        draw_line(275, 240, 265, 5);  // 90+
        draw_line(280, 240, 270, 5);
        draw_line(275, 5, 285, 240);

        draw_line(295, 240, 295, 5);  // 90
        draw_line(300, 240, 300, 5);
        draw_line(305, 5, 305, 240);

        draw_line(315, 240, 325, 5);  // 90-
        draw_line(320, 240, 330, 5);
        draw_line(335, 5, 325, 240);

        draw_line(335, 240, 550, 5);  // 45+
        draw_line(340, 240, 555, 5);
        draw_line(560, 5, 345, 240);

        draw_line(355, 240, 590, 5);  // 45
        draw_line(360, 240, 595, 5);
        draw_line(595, 10, 360, 245);

        draw_line(360, 255, 595, 40);  // 45-
        draw_line(360, 260, 595, 45);
        draw_line(595, 50, 360, 265);

        draw_line(360, 275, 590, 265); // 0+
        draw_line(360, 280, 590, 270);
        draw_line(590, 275, 360, 285);

        draw_line(360, 295, 590, 295); // 0
        draw_line(360, 300, 590, 300);
        draw_line(360, 305, 590, 305);

        draw_line(360, 315, 590, 325); // 0-
        draw_line(360, 320, 590, 330);
        draw_line(590, 335, 360, 325);

        draw_line(360, 335, 595, 550);  // -45+
        draw_line(360, 340, 595, 555);
        draw_line(595, 560, 360, 345);

        draw_line(360, 355, 595, 590);  // -45
        draw_line(360, 360, 595, 595);
        draw_line(590, 595, 355, 360);

        draw_line(345, 360, 560, 595);  // -45-
        draw_line(340, 360, 555, 595);
        draw_line(550, 595, 335, 360);

        draw_line(325, 360, 335, 595); // -90+
        draw_line(320, 360, 330, 595);
        draw_line(325, 595, 315, 360);

        draw_line(305, 360, 305, 595); // -90
        draw_line(300, 360, 300, 595);
        draw_line(295, 595, 295, 360);

        draw_line(285, 360, 275, 595); // -90-
        draw_line(280, 360, 270, 595);
        draw_line(265, 595, 275, 360);

        draw_line(265, 360, 50, 595); // -135+
        draw_line(260, 360, 45, 595);
        draw_line(40, 595, 255, 360);

        draw_line(245, 360, 10, 595); // -135
        draw_line(240, 360, 5, 595);
        draw_line(5, 590, 240, 355);

        draw_line(240, 345, 5, 560); // -135-
        draw_line(240, 340, 5, 555);
        draw_line(5, 550, 240, 335);

        auto result_texture = gl::create_texture2d(image);
        if (!result_texture)
        {
            e_debug("create texture error: {}\n", glGetError());
            return {};
        }

        return result_texture;
    }

    class main_processor
    {
    public:
        [[nodiscard]]
        bool initialize(os::module_handle_t app) noexcept
        {
            egl_ = egl_window_builder{}
                 .module(app)
                 .background(gl::colors::cyan_f)
                 .build();

            if (!egl_)
            {
                return false;
            }

            if (!lines_rendering_by_default())
            {
                return false;
            }

            if (!shaders_.initialize(sizes(egl_)))
            {
                return false;
            }

            mouse_trace_finish();

            return true;
        }

#if defined(D_OS_WINDOWS)
        std::nullopt_t operator () (const ui::mouse_double_click&) noexcept
        {
            lines_rendering_by_default();
            return std::nullopt;
        }

#endif

        std::nullopt_t operator () (const ui::mouse_move_event& e) noexcept
        {
#if  defined(D_OS_WINDOWS)
            if (!e.keys().is_left())
            {
                mouse_trace_finish();
                return std::nullopt;
            }
#endif

            if (1u == e.size())
            {
                const auto new_pos = e.pointer(0);
                if (invalid_mouse_pos != new_pos)
                {
                    const auto old_pos = std::exchange(mouse_pos_, new_pos);

                    if (invalid_mouse_pos != old_pos)
                    {
                        const auto d_mouse = as_signed(new_pos - old_pos);

                        const auto new_position = position_ + point2d
                        {
                            trunc_cast<pxoff_t>(d_mouse.x()),
                            trunc_cast<pxoff_t>(d_mouse.y())
                        };

                        lines_rendering(new_position);
                    }
                }
            }

            return std::nullopt;
        }

        std::nullopt_t operator () (const ui::mouse_up_event&) noexcept
        {
            mouse_trace_finish();
            return std::nullopt;
        }

        ui::milliseconds_t operator () (ui::idle_event) noexcept
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
            const egl_painting_owner painting_lock{ egl_ };
            shaders_.draw(texture_);
        }

        static constexpr auto invalid_mouse_pos = fill_to<point2d>(numeric_max_v<ui::pointer_event::value_type>);

        constexpr void mouse_trace_finish() noexcept
        {
            mouse_pos_ = invalid_mouse_pos;
        }

        bool lines_rendering(const pxoff2d position) noexcept
        {
            if (texture_ && position == position_)
            {
                return true;
            }

            if (image_)
            {
                zero_memory(image_);
            }
            else
            {
                image_ = pix8map{ 600_px, 600_px };
                if (!image_)
                {
                    e_debug("out of memory");
                    return false;
                }
            }

            auto new_texture = ::lines_rendering(image_, position);
            if (!new_texture)
            {
                return false;
            } 
             
            texture_ = std::move(new_texture);
            position_ = position;
            need_redraw_ = true;
            return true;
        }

        bool lines_rendering_by_default() noexcept
        {
            return lines_rendering({ 0_px, 0_px });
        }

    private:
        class shaders_lib
        {
        public:
            bool initialize(pxsize2d viewport) noexcept
            {
                if (!lib.build())
                {
                    return false;
                }

                lib.use();
                lib.vert.u_viewport.store(viewport);
                lib.vert.u_position.store(100_px, 150_px);
                return true;
            }

            void draw(gl::texture2d_resources texture) const noexcept
            {
                lib.use();
                lib.frag.s_texture.store(texture);
                lib.vert.u_size.store(sizes(texture));
                lib.vert.a_frame.draw();
            }

        private:
            shaders_library<vert::positioned_texture, frag::inverted_texture> lib{};
        };

        egl_window egl_{};
        shaders_lib shaders_{};
        gl::texture2d texture_{};
        ui::pointer_event::point2d_type mouse_pos_{};
        pxoff2d position_{};
        pix8map image_{};
        bool need_redraw_{ true };
    };
}


int app_main(os::module_handle_t app) noexcept
{
    main_processor processor;

    if (!processor.initialize(app))
    {
        e_debug("create window error: ui error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return processor.run();
}




