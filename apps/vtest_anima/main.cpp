#include <core/lerp.h>

#include <debug/debug.h>

#include <gl/draw.h>
#include <egl/event_loop.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace
{
    constexpr auto anima_start_color = colors::yellow;
    constexpr auto anima_end_color = colors::black;

    using duration_t = ui::milliseconds_t;
    using duration_rep_t = duration_t::rep;

    constexpr duration_t anima_lerp_period{ 2s };
    constexpr duration_t anima_working_period{ 6 * anima_lerp_period };
    constexpr duration_t anima_paused_period{ anima_working_period };

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
        constexpr auto period = anima_lerp_period.count();

        constexpr auto anima_lerp = lerp
        (
            num_range{ duration_t::zero().count(), period },
            num_range{ anima_start_color, anima_end_color }
        );

        return color_cast<rgba_colorf_t>(anima_lerp(oscillating_time(now.count(), period)));
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

        u_color.store(anima_color(now + anima_lerp_period));

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
        egl_t egl;

        void draw(duration_t now) const noexcept
        {
            egl::painting_owner painting_lock{ egl };
            draw_figure(now);
        }

        ui::milliseconds_t operator () (ui::idle_event) const noexcept
        {
            constexpr auto anima_period = anima_working_period + anima_paused_period;

            const auto now = steady_clock::now();

            static const auto anima_start_time = now;

            const auto anima_time = duration_cast<duration_t>(now - anima_start_time) % anima_period;

            const auto is_anima = anima_time <= anima_working_period;

            if (is_anima)
            {
                draw(anima_time);
            }

            return (is_anima) ? 0ms : (anima_period - anima_time);
        }
    };
}

int app_main(os::module_handle_t app) noexcept
{
    const main_processor processor
    {
        .egl{ egl::instance(app) }
    };

    if (!processor.egl)
    {
        e_debug("create window error: ui error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return run(processor.egl, processor);
}

