#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/egl_ui_owner.h>


namespace
{
    struct engine
    {
        egl_ui_owner egl;
        bool need_redraw{ true };

        constexpr explicit operator bool() const noexcept
        {
            return !!egl;
        }

        void draw() const noexcept
        {
            static const auto program = gl::create_program
            (
                R"(
                        attribute vec2 a_position;    
                        void main()
                        {
                            gl_Position = vec4(a_position, 0.0, 1.0);
                        }
                    )"_glsl,
                R"(
                        precision mediump float;
                        void main()
                        {
                            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                        }
                    )"_glsl
            );

            static const auto a_position = gl::get_attribute_location(program, "a_position"_zsv);

            const egl_painting_owner painting_owner{ egl };
            gl::viewport(egl.viewport());
            gl::clear(colors::white_f);
            gl::use(program);

            constexpr GLfloat radius{ 0.25f };
            constexpr GLfloat dia{ 2 * radius };
            constexpr GLfloat x_left{ -1.0f };
            constexpr GLfloat y_top{ 1.0f };
            constexpr GLfloat y_bottom{ y_top - dia };

            constexpr gl::vec2f vertices[]
            {
                { x_left + radius, y_top },
                { x_left, y_bottom },
                { x_left + dia, y_bottom }
            };

            gl::set_vertex_pointer(a_position, vertices);
            gl::draw_arrays(gl::draw_mode::triangles, 0, std::size(vertices));
        }

        void operator () (ui::content_rect_changed_event) noexcept
        {
            egl_.update_viewport();
        }

        void operator () (ui::redraw_needed_event) noexcept
        {
            need_redraw = true;
        }

        ui::milliseconds operator () (ui::idle_event) noexcept
        {
            if (need_redraw)
            {
                need_redraw = false;
                draw();
            }

            return ui::infinite;
        }
    };
}

int app_main(os::module_handle_t app) noexcept
{
    engine engine{ .egl{ create_egl_ui(app) } };
    if (!engine)
    {
        return EXIT_FAILURE;
    }

    return ui::run_event_loop(engine.egl, engine);
}
