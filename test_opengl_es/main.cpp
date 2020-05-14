#include <string_view>

#include <platform/gl/shader.h>
#include <platform/gl/shaders_program.h>

#include <platform/windows/debug.h>
#include <platform/windows/window.h>
#include <platform/windows/event_matching.h>
#include <platform/windows/event_loop.h>
#include <platform/egl/egl_window.h>

using namespace std::literals;
using namespace os_windows;

namespace
{
    struct shader_source
    {
        std::string_view source;
        gl::shader_type type;
    };

    constexpr shader_source shaders[] =
    {
        {
            R"(
                attribute vec4 vPosition;
                void main()
                {
                    gl_Position = vPosition;
                }
            )"sv, gl::shader_type::vertex
        },
        {
            R"(
                precision mediump float;
                void main()
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                }
            )"sv, gl::shader_type::fragment
        }
    };

    gl::safe_shaders_program compile_shaders_program(span<const shader_source> sources) noexcept
    {
        if ( auto program = gl::shaders_program::create_program() )
        {
            for ( const auto& source : sources )
            {
                if ( const auto shader = create_shader(source.type) )
                {
                    if ( compile(shader, source.source) )
                    {
                        attach_shader(program, shader);
                        continue;
                    }
                }

                program = {};
                break;
            }

            if ( program && link(program) )
            {
                return program;
            }
        }

        return {};
    }

    struct main_event_processor
    {
        safe_window app_window;
        safe_window render_window;

        egl::safe_window_context egl_window_context;
        gl::safe_shaders_program gl_shaders_program;

        void draw() const noexcept
        {
            constexpr GLfloat vertices[] =
            {
                0.0f, 0.5f,
                -0.5f, -0.5f,
                0.5f, -0.5f,
            };
            
            {
                const auto view_size = client_rect(app_window).size();
                const auto render_size = client_rect(render_window).size();
                glViewport
                (
                    0,
                    render_size.height() - view_size.height(),
                    view_size.width(),
                    view_size.height()
                );
            }

            glClear(GL_COLOR_BUFFER_BIT);
            
            use(gl_shaders_program);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
            glEnableVertexAttribArray(0);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            swap_buffers(egl_window_context);
        }

        event_result_t operator () (const close_event& e) const noexcept
        {
            PostQuitMessage(0);
            return 0L;
        }

        bool operator () () const noexcept
        {
            draw();
            return false;
        }
    };
}


int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show_command)
{
    main_event_processor processor;

    processor.app_window =
         window_factory{}
        .title(L"test_win32app")
        .type
        (
             window_type_factory{}
            .name(L"test_win32wnd")
            .background(stock_brush::white)
            .module_address(instance)
            .create()
        )
        .create();

    if ( !processor.app_window )
    {
        output_debug_string("create window error {}", GetLastError());
        return -1;
    }

    processor.render_window =
        window_factory{}
        .parent(processor.app_window)
        .type
        (
            window_type_factory{}
            .name(L"test_win32subwnd")
            .style(CS_OWNDC)
            .background(stock_brush::white)
            .module_address(instance)
            .create()
        )
        .rect(native_window_system::full_rect(::GetDesktopWindow()))
        .create();

    if ( !processor.render_window )
    {
        output_debug_string("create render window error {}", GetLastError());
        return -1;
    }

    processor.egl_window_context = egl::window_context::create_context(processor.render_window->handle());
    if ( !processor.egl_window_context )
    {
        output_debug_string("egl error: {}\n", eglGetError());
        return -1;
    }

    processor.gl_shaders_program = compile_shaders_program(shaders);
    if ( !processor.gl_shaders_program )
    {
        output_debug_string("gl error: {}\n", glGetError());
        return -1;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    const auto process_owner = attach_event_processor(processor.app_window, event_match(std::ref(processor)));

    show(processor.app_window, show_command);

    return run_event_loop(std::ref(processor));
}

