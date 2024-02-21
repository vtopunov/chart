#include <random>
#include <variant>

#include <debug/debug.h>

#include <utility/shader_library.h>

#include <widget/run.h>


using namespace std::chrono_literals;

namespace
{
    class simple_widget
    {
        static constexpr auto invalid_mouse_pos = fill_to<point2d>(numeric_max_v<ui::pointer_event::value_type>);

    public:
        [[nodiscard]]
        bool initialize(pxsize2d viewport) noexcept
        {
            if (!lib_.build())
            {
                return false;
            }

            lib_.use();
            lib_.vert.u_viewport.store(viewport);
            lib_.frag.u_color.store(colors::green_f);
            lib_.frag.u_width.store(1.0f, 1.0f);
            lib_.frag.u_begin.store(11.0f, 11.0f);
            lib_.frag.u_repeat.store(66.0f, 66.0f);
            return true;
        }

        void update_content_sizes(pxsize2d content_sizes) noexcept
        {
            lib_.use();

            {
                constexpr auto scale = 0.67f;
                constexpr auto relative_shift = (0.5f - 0.5f * scale);

                const auto position = relative_shift * content_sizes;
                const auto rect_sizes = scale * content_sizes;
                lib_.vert.u_position.store(position);
                lib_.vert.u_size.store(rect_sizes);
            }
        }

        bool operator () (viewport_size2d viewport) noexcept
        {
            return initialize(viewport);
        }

        widget::event_result operator () (const ui::size_event& e)
        {
            update_content_sizes(e.sizes());
            return widget::event_result::redraw;
        }

        void operator () (widget::redraw_event<>) const noexcept
        {
            lib_.use();
            lib_.vert.a_frame.draw();
        }

        constexpr widget::noapply_t apply(no_overload) const noexcept
        {
            return widget::noapply;
        }

    private:
        struct fragment_shader
        {
            static constexpr auto shader_text = R"(
                precision mediump float;

                uniform vec2 u_position;
                uniform vec2 u_size;
                uniform vec2 u_viewport;
                uniform vec4 u_color;
                uniform vec2 u_width;
                uniform vec2 u_begin;
                uniform vec2 u_repeat;

                void main()
                {
                    vec2 px_begin = vec2(u_position.x, u_viewport.y - u_size.y - u_position.y) + u_begin - 0.5 * u_width + 0.5;
                    vec2 a = step(u_width, mod(gl_FragCoord.xy - px_begin, u_repeat));
                    gl_FragColor = (1.0 - a.x * a.y) * u_color;
                }
            )"_glsl;

            gl::uniform_vec4f u_color = gl::invaliduniform;
            gl::uniform_vec2f u_width = gl::invaliduniform;
            gl::uniform_vec2f u_begin = gl::invaliduniform;
            gl::uniform_vec2f u_repeat = gl::invaliduniform;

            template<class Serializer>
            constexpr void serialize(Serializer& ser) noexcept
            {
                ser(u_color, "u_color"_zsv);
                ser(u_width, "u_width"_zsv);
                ser(u_begin, "u_begin"_zsv);
                ser(u_repeat, "u_repeat"_zsv);
            }
        };

        shader_library<vert::positioned_rectangle, fragment_shader> lib_{};
    };

    class main_widget
    {
    public:
        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(widget_);
        }

    private:
        simple_widget widget_{};
    };
}

int app_main(os::module_handle_t app) noexcept
{
    auto window = widget::window_builder{}
        .module(app)
        .sizes(1500_npx, 1534_npx)
        .command_show(ui::show_command::normal)
        .build();

    return widget::run<main_widget>(window);
}




