#include <core/lerp.h>

#include <debug/debug.h>

#include <gl/draw.h>

#include <egl_ui/egl_ui_owner.h>

using namespace std::chrono;
using namespace std::chrono_literals;


namespace
{
    constexpr auto anima_start_color = colors::yellow;
    constexpr auto anima_end_color = colors::black;

    using duration_t = ui::milliseconds;
    using duration_rep_t = duration_t::rep;

    constexpr duration_t anima_lerp_period{ 2s };
    constexpr duration_t anima_working_period{ 6 * anima_lerp_period };
    constexpr duration_t anima_paused_period{ anima_working_period };
    constexpr auto anima_period = anima_working_period + anima_paused_period;

    [[nodiscard]]
    constexpr duration_rep_t oscillating_time(duration_rep_t time, duration_rep_t period) noexcept
    {
        const auto count = time / period;
        const auto oscillating_bit = count & 1;
        const auto oscillating_sign = 1 - 2 * oscillating_bit;
        const auto bound = (count + oscillating_bit) * period;
        return oscillating_sign * (time - bound);
    }

    [[nodiscard]]
    constexpr rgbaf_color_t anima_color(duration_t now) noexcept
    {
        constexpr auto period = anima_lerp_period.count();

        constexpr auto anima_lerp = lerp
        (
            duration_t::zero().count(), period,
            anima_start_color, anima_end_color
        );

        return to_colorf(anima_lerp(oscillating_time(now.count(), period)));
    }

    void draw_figure(pxsize2d viewport, rgbaf_color_t color) noexcept
    {
        static const auto shaders = gl::create_program
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

        gl::viewport(viewport);
        gl::clear(colors::white_f);
        gl::use(shaders);

        u_color.store(color);

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

    class anima_timer
    {
    public:
        using clock_t = steady_clock;
        using time_point_t = clock_t::time_point;

        duration_t operator () () noexcept
        {
            const auto now = steady_clock::now();
            if (invalid_time == start_time_)
            {
                start_time_ = now;
            }

            return duration_cast<duration_t>(now - start_time_) % anima_period;
        }

    private:
        static constexpr auto invalid_time = time_point_t::min();

    private:
        time_point_t start_time_{ invalid_time };
    };

    struct main_processor
    {
        egl_ui_owner egl;
        anima_timer timer;
        rgbaf_color_t color{ to_colorf(anima_start_color) };
        bool force_redraw{ true };

        void draw() const noexcept
        {
            const egl_painting_owner painting_owner{ egl };
            draw_figure(viewport(egl), color);
        }

        void operator () (ui::content_rect_changed_event) noexcept
        {
            D_UNUSED(egl.update_viewport());
        }

        void operator () (ui::redraw_needed_event) noexcept
        {
            force_redraw = true;
        }

        [[nodiscard]]
        ui::milliseconds operator () (ui::idle_event) noexcept
        {
            const auto anima_time = timer();
            const auto is_anima = anima_time <= anima_working_period;

            if (is_anima)
            {
                color = anima_color(anima_time);
            }

            if (is_anima || force_redraw)
            {
                force_redraw = false;
                draw();
            }

            return (is_anima) ? 0ms : (anima_period - anima_time);
        }
    };
}

int app_main(os::module_handle_t app) noexcept
{
    main_processor processor
    {
        .egl{ create_egl_ui(app) }
    };

    if (!processor.egl)
    {
        e_debug("create window error: ui error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return ui::run_event_loop(processor.egl, processor);
}