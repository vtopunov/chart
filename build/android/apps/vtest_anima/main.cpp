#include <cassert>

#include <android_native_app_glue.h>

#include <gl/draw.h>
#include <egl/instance.h>

#include <debug/debug.h>


using namespace std::string_view_literals;

namespace
{
    enum class message_type : int32_t
    {
        invalid = -1,
        window_destroyed = APP_CMD_TERM_WINDOW
    };

    struct message
    {
        message_type type;
    };

    GLfloat anima_gen() noexcept
    {
        static uint8_t cnt{ 0 };
        return ((++cnt) & 0x7f) / 128.0f;
    }

    void engine_draw_frame(const egl_resources& egl)
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
    }

    int32_t engine_handle_input(android_app* app, AInputEvent* event)
    {
        return 0;
    }

    void engine_handle_cmd(android_app* app, int32_t cmd)
    {
        static_cast<message*>(app->userData)->type = underlying_cast<message_type>(cmd);
    }
}

int app_main(os::module_handle_t app)
{
    debug("android_main");
    const auto egl = egl::instance(app);
    if (!egl)
    {
        e_debug("egl error {}", eglGetError());
        return EXIT_FAILURE;
    }

    message msg{ message_type::invalid };
    app->userData = std::addressof(msg);
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;

    int events{ 0 };
    android_poll_source* source{ nullptr };

    while (!(app->destroyRequested))
    {
        engine_draw_frame(egl);

        if (const auto ident = ALooper_pollAll(0, nullptr, &events, (void**)&source); ident >= 0)
        {
            if (source)
            {
                source->process(app, source);

                if (message_type::window_destroyed == msg.type)
                {
                    quit(egl);
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
