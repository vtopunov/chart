#include <cassert>

#include <android_native_app_glue.h>

#include <gl/draw.h>
#include <egl/event_loop.h>

#include <debug/debug.h>


using namespace std::string_view_literals;

namespace
{
    GLfloat anima_gen() noexcept
    {
        static uint8_t cnt{ 0 };
        return ((++cnt) & 0x7f) / 128.0f;
    }

    struct main_processor
    {
        egl_t egl;

        ui::milliseconds_t operator () (ui::idle_event) const noexcept
        {
            static const auto shaders = gl::create_shaders_program
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

            static const auto a_position = gl::get_attribute_location(shaders, "a_position"_zsv);

            egl::painting_owner own{ egl };

            const auto angle = anima_gen();
            gl::clear(angle, angle, angle);

            gl::use(shaders);

            constexpr gl::vec2f vertices[]
            {
                { 0.0f, 0.5f },
                { -0.5f, -0.5f },
                { 0.5f, -0.5f }
            };

            gl::set_vertex_pointer(a_position, vertices);

            gl::draw_arrays(gl::draw_mode::triangles, 0, std::size(vertices));

            return ui::milliseconds_t::zero();
        }
    };
}

int app_main(os::module_handle_t app)
{
    main_processor processor
    {
        .egl{ egl::instance(app) }
    };

    if (!processor.egl)
    {
        e_debug("egl error {}", eglGetError());
        return EXIT_FAILURE;
    }

    return run(processor.egl, processor);
}
