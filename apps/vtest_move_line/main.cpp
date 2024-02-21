#include <debug/debug.h>

#include <px/algorithm.h>
#include <px/pixmap.h>

#include <egl_ui/egl_ui_owner.h>

#include <utility/shader_library.h>

#include "../vtest_line/test_figure.h"


namespace
{
    gl::texture2d lines_rendering(const pix8span image, const pxoff2d d) noexcept
    {
        {
            const auto dd = md_narrow<px::real_point2d>(d);
            for (const auto& line : vtest_line_figure::figure)
            {
                draw_antialiasing_line
                (
                    image,
                    line.x0 + dd.x(),
                    line.y0 + dd.y(),
                    line.x1 + dd.x(),
                    line.y1 + dd.y()
                 );
            }
        }

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
        static constexpr auto background_color = colors::cyan;
        static constexpr auto gl_background_color_f = to_colorf(background_color);

    public:
        [[nodiscard]]
        bool initialize(os::module_handle_t app) noexcept
        {
#ifdef D_OS_WINDOWS
            {
                auto brush = ui::create_brush(background_color);
                if (!brush)
                {
                    return false;
                }

                egl_ = egl_ui_builder{}
                    .module(app)
                    .background(std::move(brush))
                    .build();
            } 
            
#else
            egl_ = create_egl_ui(app);

#endif

            if (!egl_)
            {
                return false;
            }

            if (!lines_rendering_by_default())
            {
                return false;
            }

            if (!shaders_.initialize(egl_.viewport))
            {
                return false;
            }

            mouse_trace_finish();
            return true;
        }

#if defined(D_OS_WINDOWS)
        std::nullopt_t operator () (const ui::mouse_double_click_event&) noexcept
        {
            D_ASSERT_OR_UNUSED(lines_rendering_by_default());
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
                        D_ASSERT_OR_UNUSED(lines_rendering(position_ + md_trunc_cast<pxoff2d>(as_signed(new_pos - old_pos))));
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
            gl::clear(gl_background_color_f);

            shaders_.draw(texture_);
        }

        static constexpr auto invalid_mouse_pos = fill_to<point2d>(numeric_max_v<ui::pointer_event::value_type>);

        constexpr void mouse_trace_finish() noexcept
        {
            mouse_pos_ = invalid_mouse_pos;
        }

        [[nodiscard]]
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
                image_ = pix8map{ 600_npx, 600_npx };
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

        [[nodiscard]]
        bool lines_rendering_by_default() noexcept
        {
            return lines_rendering({ 0_npxz, 0_npxz });
        }

    private:
        class shaders_lib
        {
        public:
            [[nodiscard]]
            bool initialize(pxsize2d viewport) noexcept
            {
                if (!lib.build())
                {
                    return false;
                }

                lib.use();
                lib.vert.u_viewport.store(viewport);
                lib.vert.u_position.store(100_npx, 150_npx);
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
            shader_library<vert::positioned_texture, frag::inverted_texture> lib{};
        };

        egl_ui_owner egl_{};
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




