#include <core/color.h>
#include <core/debug.h>

#include <display/event_loop.h>
#include <display/gl/draw.h>
#include <display/egl/egl_window.h>

#include <apps/utility/utility.h>

using namespace std::string_view_literals;
using namespace display::gl_literals;
using namespace display;


namespace
{
    void draw_texture_mix(gl::texture_resource2d_t base_texture, gl::texture_resource2d_t mix_texture) noexcept
    {
        static const auto shaders = gl::create_shaders_program
        (
            R"(
                attribute vec2 a_position;
                attribute vec2 a_texture;

                varying vec2 v_texture;

                void main()
                {
                    gl_Position = vec4(a_position, 0.0, 1.0);
                    v_texture = a_texture;
                }
            )"_glsl,
            R"(
                precision mediump float;

                uniform sampler2D s_base_texture;
                uniform sampler2D s_mix_texture;
                varying vec2 v_texture;

                void main()
                {                
                    vec4 base_color;
                    vec4 mix_color;

                    base_color = texture2D(s_base_texture, v_texture);
                    mix_color = texture2D(s_mix_texture, v_texture);
                    gl_FragColor = base_color * (mix_color + 0.25);
                }
           )"_glsl
        );

        static const gl::attribute_location attributes[] =
        {
            gl::get_attribute_location(shaders, "a_position"_zsv),
            gl::get_attribute_location(shaders, "a_texture"_zsv)
        };

        static const auto s_base_texture = gl::get_texture_sampler2D(shaders, "s_base_texture"_zsv);
        static const auto s_mix_texture = gl::get_texture_sampler2D(shaders, "s_mix_texture"_zsv);

        gl::clear(colors::white_f);

        gl::use(shaders);

        constexpr GLfloat radius{ 0.25f };
        
        constexpr gl::vertex<gl::vec2f, gl::vec2f> vertices[]
        {
            { {-radius,  radius}, {0.0f, 0.0f} },
            { {-radius, -radius}, {0.0f, 1.0f} },
            { {radius,  radius}, {1.0f, 0.0f} },
            { {radius, -radius}, {1.0f, 1.0f} }
        };

        static const gl::vertex_buffer vbo{ vertices };

        vbo.bind(attributes);

        base_texture.bind(s_base_texture);
        mix_texture.bind(s_mix_texture);

        vbo.draw(gl::draw_mode::triangle_strip);
    }

    class main_processor
    {
    public:
        constexpr main_processor() noexcept = default;

        bool initialize() noexcept
        {
            egl_window_ = egl_window_factory{}.create();
            if (!egl_window_)
            {
                output_debug_string("create window error: window error: {}, egl error: {}\n",
                    display::last_error_code(), eglGetError());
                return false;
            }
            
            base_texture_ = png_texture(_PATH("base.png"));
            if (!base_texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            mix_texture_ = png_texture(_PATH("mix.png"));
            if (!mix_texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            return true;
        }

        void show(int command_show) noexcept
        {
            need_redraw_ = true;
            display::show(egl_window_, command_show);
        }

        void draw() const noexcept
        {
            [[maybe_unused]]
            const auto lock = egl_window_->begin();

            draw_texture_mix(base_texture_, mix_texture_);
        }

        bool operator () (peek_event) const noexcept
        {
            return need_redraw_;
        }
        
        void operator () (idle_event) noexcept
        {
            need_redraw_ = false;
            draw();
        }

        int run() noexcept
        {
            return run_event_loop(egl_window_, *this);
        }

    private:
        egl_window_t egl_window_;
        gl::texture2d_t base_texture_;
        gl::texture2d_t mix_texture_;
        bool need_redraw_{ false };
    };
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int command_show)
{
    main_processor processor;

    if (!processor.initialize())
    {
        output_debug_string("initialize fail\n");
        return -1;
    }

    processor.show(command_show);

    return processor.run();
}




