#include <core/color.h>
#include <core/debug.h>

#include <ui/event_loop.h>
#include <gl/draw.h>
#include <egl/window.h>

#include <apps/utility/png.h>

using namespace std::string_view_literals;
using namespace gl_literals;

namespace
{
    void draw_texture_mix(gl::texture2d_resource base_texture, gl::texture2d_resource mix_texture) noexcept
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

        static const auto attributes = gl::get_attribute_locations(shaders, "a_position"_zsv, "a_texture"_zsv);
        static const auto s_base_texture = gl::texture_sampler2D_t::instance(shaders, "s_base_texture"_zsv);
        static const auto s_mix_texture = gl::texture_sampler2D_t::instance(shaders, "s_mix_texture"_zsv);

        gl::clear(colors::white_f);

        gl::use(shaders);

        constexpr GLfloat radius{ 0.25f };
        
        constexpr gl::vertex<gl::vec2f_t, gl::vec2f_t> vertices[]
        {
            { {-radius,  radius}, {0.0f, 0.0f} },
            { {-radius, -radius}, {0.0f, 1.0f} },
            { {radius,  radius}, {1.0f, 0.0f} },
            { {radius, -radius}, {1.0f, 1.0f} }
        };

        static const gl::vertex_buffer vbo{ vertices };

        s_base_texture.store(base_texture);
        s_mix_texture.store(mix_texture);

        vbo.bind(attributes).draw(gl::draw_mode::triangle_strip);

        // {
        //   constexpr GLubyte indices[]{ 0, 2, 1, 1, 2, 3 };
        //   gl::draw_elements(gl::draw_mode::triangles, indices);
        // }
    }

    class main_processor
    {
    public:
        constexpr main_processor() noexcept = default;

        bool initialize() noexcept
        {
            egl_ = egl::window_factory{}.create();
            if (!egl_)
            {
                output_debug_string("create window error: window error: {}, egl error: {}\n",
                    ui::error_code(), eglGetError());
                return false;
            }
            
            png_reader png;

            base_texture_ = png.texture_from_file(_PATH("base.png"));
            if (!base_texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            mix_texture_ = png.texture_from_file(_PATH("mix.png"));
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
            ui::show(egl_, command_show);
        }

        void draw() const noexcept
        {
            if (const auto lock = egl::begin_painting(egl_))
            {
                draw_texture_mix(base_texture_, mix_texture_);
            }
        }

        bool operator () (ui::peek_event) const noexcept
        {
            return need_redraw_;
        }
        
        void operator () (ui::idle_event) noexcept
        {
            need_redraw_ = false;
            draw();
        }

        int run() noexcept
        {
            return ui::run_event_loop(egl_, *this);
        }

    private:
        egl::window_t egl_;
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




