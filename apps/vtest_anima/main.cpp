#include <core/lerp.h>

#include <debug/debug.h>

#include <ui/timer.h>
#include <gl/draw.h>
#include <egl/event_loop.h>

using namespace std::chrono_literals;
using namespace std::chrono;

namespace
{
    constexpr auto anima_start_color = colors::yellow;
    constexpr auto anima_end_color = colors::black;
    constexpr auto anima_pause_color = anima_start_color;

    using duration_t = milliseconds;
    using duration_rep_t = duration_t::rep;

    constexpr duration_t anima_period{2s};
    constexpr duration_t anima_working_time{6 * anima_period};
    constexpr duration_t anima_paused_time{anima_working_time};

    constexpr duration_rep_t oscillating_time(duration_rep_t time, duration_rep_t period) noexcept
    {
        const auto count = time / period;
        const auto oscillating_bit = count & 1;
        const auto oscillating_sign = 1 - 2 * oscillating_bit;
        const auto bound = (count + oscillating_bit) * period;
        return oscillating_sign * (time - bound);
    }

    constexpr rgba_colorf_t anima_color(duration_t now) noexcept
    {
        constexpr auto anima_period_rep = anima_period.count();

        constexpr auto anima_lerp = lerp
        (
            num_range{duration_t::zero().count(), anima_period_rep},
            num_range{anima_start_color, anima_end_color}
        );

        return color_cast<rgba_colorf_t>(anima_lerp(oscillating_time(now.count(), anima_period_rep)));
    }

    void draw_figure(duration_t now) noexcept
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
                uniform vec4 u_color;

                void main()
                {
                    gl_FragColor = u_color;
                }
           )"_glsl
        );

        static const auto a_position = gl::get_attribute_location(shaders, "a_position"_zsv);
        static const auto u_color = gl::uniform_vec4f::instance(shaders, "u_color"_zsv);

        gl::clear(anima_color(now));

        gl::use(shaders);

        u_color.store(anima_color(now + anima_period));

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

    struct main_processor
    {
        egl::window egl;

        ui::timer anima_wakeup_timer;

        void draw(duration_t now) const noexcept
        {
            if (const auto lock = begin_painting(egl))
            {
                draw_figure(now);
            }
        }

        void operator () (ui::timer_event e)  noexcept
        {
            if (e.is(anima_wakeup_timer))
            {
                anima_wakeup_timer.reset();
            }
        }

        bool operator () (ui::idle_event)  noexcept
        {
            constexpr auto period = anima_working_time + anima_paused_time;

            const auto now = steady_clock::now();

            static const auto start_time = now;

            const auto duration_now = duration_cast<duration_t>(now - start_time) % period;

            const auto is_anima = duration_now <= anima_working_time;

            if (is_anima)
            {
                draw(duration_now);
            }
            else
            {
                if (!anima_wakeup_timer)
                {
                    const auto anima_wakeup_time = period - duration_now;
                    anima_wakeup_timer = ui::create_timer(anima_wakeup_time);
                    D_ASSERT(anima_wakeup_timer);
                    draw(anima_working_time);
                }
            }

            return is_anima;
        }
    };
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int command_show)
{
    main_processor processor
    {
        .egl
        {
            egl::window_factory{}
            .title(L"hello triangle")
            .window_type
            (
                ui::type_window_factory{}
                .module_instance(instance)
                .create()
            )
            .create()
        }
    };

    if (!processor.egl)
    {
        e_debug("create window error: window error: {}, egl error: {}", 
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    show(processor.egl, command_show);

    return run_event_loop(processor.egl, processor);
}

