#include <string_view>

#include <core/color.h>
#include <core/lerp.h>

#include <platform/gl/shader.h>
#include <platform/gl/shaders_program.h>

#include <platform/windows/debug.h>
#include <platform/windows/window.h>
#include <platform/windows/event_matching.h>
#include <platform/windows/event_loop.h>
#include <platform/windows/event_timer.h>
#include <platform/egl/egl_window.h>

using namespace std::literals;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace os_windows;

namespace
{
    using argb_color_gl_t = argb_color<GLfloat>;

    constexpr auto default_clear_color = colors::white;
    constexpr auto anima_start_clear_color = colors::white;
    constexpr auto anima_end_clear_color = colors::black;

    using anima_duration_t = milliseconds;

    constexpr anima_duration_t anima_period{ 2s };
    constexpr anima_duration_t anima_working_time{ 6 * anima_period };
    constexpr anima_duration_t anima_paused_time{ anima_working_time };

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
        if (auto program = gl::shaders_program::create_program())
        {
            for (const auto& source : sources)
            {
                if (const auto shader = create_shader(source.type))
                {
                    if (compile(shader, source.source))
                    {
                        attach_shader(program, shader);
                        continue;
                    }
                }

                program = {};
                break;
            }

            if (program && link(program))
            {
                return program;
            }
        }

        return {};
    }

    void set_clear_color(argb_color_gl_t color) noexcept
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    template<class T>
    constexpr T oscillating_time(T time, T period) noexcept
    {
        const auto count = time / period;
        const auto oscillating_bit = count & 1;
        const auto oscillating_sign = 1 - 2 * oscillating_bit;
        const auto bound = (count + oscillating_bit) * period;
        return oscillating_sign * (time - bound);
    }

    constexpr argb_color_gl_t anima_clear_color(const anima_duration_t now) noexcept
    {
        constexpr auto anima_period_rep = anima_period.count();

        constexpr auto anima_lerp = lerp
        (
            num_range{ anima_duration_t::zero().count(), anima_period_rep },
            num_range{ anima_start_clear_color, anima_end_clear_color }
        );

        return color_cast<argb_color_gl_t>(anima_lerp(oscillating_time(now.count(), anima_period_rep)));
    }

    void draw_figure() noexcept
    {
        constexpr size_t dimension{ 2u };

        constexpr GLfloat vertices[] =
        {
            0.0f, 0.5f,
            -0.5f, -0.5f,
            0.5f, -0.5f
        };

        glVertexAttribPointer(0, narrow_cast<GLuint>(dimension), GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(0);

        constexpr auto count = std::size(vertices) / dimension;

        glDrawArrays(GL_TRIANGLES, 0, narrow_cast<GLsizei>(count));
    }

    struct main_event_processor
    {
        safe_window app_window;
        safe_window render_window;

        egl::safe_window_context egl_window_context;
        gl::safe_shaders_program gl_shaders_program;

        safe_event_timer anima_timer;
        bool need_redraw = false;

        void draw() const noexcept
        {
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

            draw_figure();

            swap_buffers(egl_window_context);
        }

        event_result_t operator () (const close_event& e) const noexcept
        {
            PostQuitMessage(0);
            return 0L;
        }

        event_result_t operator () (const size_event&) noexcept
        {
            need_redraw = true;
            return 0L;
        }

        bool operator () (const peek_event&) const noexcept
        {
            const auto now = duration_cast<anima_duration_t>(steady_clock::now().time_since_epoch());
            const auto pause_looped_time = now % (anima_working_time + anima_paused_time);
            const auto is_anima = pause_looped_time <= anima_working_time;
            const auto is_draw = is_anima || need_redraw;

            if (is_draw)
            {
                const auto anima_time = (is_anima) ? now : (now - pause_looped_time + anima_working_time);
                set_clear_color(anima_clear_color(anima_time));
            }

            return is_draw;
        }

        void operator () (const idle_event&) noexcept
        {
            need_redraw = false;
            draw();
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

    if (!processor.app_window)
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
            .background(stock_brush::white)
            .module_address(instance)
            .create()
        )
        .rect(native_window_system::full_rect(::GetDesktopWindow()))
        .create();

    if (!processor.render_window)
    {
        output_debug_string("create render window error {}", GetLastError());
        return -1;
    }

    processor.egl_window_context = egl::window_context::create_context(processor.render_window->handle());
    if (!processor.egl_window_context)
    {
        output_debug_string("egl error: {}\n", eglGetError());
        return -1;
    }

    processor.gl_shaders_program = compile_shaders_program(shaders);
    if (!processor.gl_shaders_program)
    {
        output_debug_string("gl error: {}\n", glGetError());
        return -1;
    }

    processor.anima_timer = event_timer::create_timer(processor.app_window, anima_period / 5);

    const auto process_owner = attach_event_processor(processor.app_window, event_match(std::ref(processor)));

    show(processor.app_window, show_command);

    return run_event_loop(std::ref(processor));
}

