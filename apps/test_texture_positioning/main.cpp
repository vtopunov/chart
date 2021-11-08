#include <core/color.h>
#include <core/debug.h>

#include <ui/event_loop.h>
#include <gl/draw.h>
#include <egl/window.h>

#include <apps/utility/utility.h>

using namespace std::string_view_literals;
using namespace gl_literals;


namespace
{
    class main_processor
    {
    public:
        constexpr main_processor() noexcept = default;

        D_DISABLE_COPY_MOVE(main_processor);

        using vertext_type = gl::vec2f_t;

        bool initialize() noexcept
        {
            egl_window_ = egl::window_factory{}.create();
            if (!egl_window_)
            {
                output_debug_string("create window error: window error: {}, egl error: {}\n",
                    ui::error_code(), eglGetError());
                return false;
            }

            if (const auto resolution = ui::display_resolution(); egl_window_->viewport != resolution)
            {
                output_debug_string("Instance of window is not high dpi. Add <dpiAware>true</dpiAware> in manifest. Resolution: {}x{}", resolution.width(), resolution.height());
                return false;
            }

            texture_ = png_reader{}.texture_from_file(_PATH("grid_9x11.png"));
            if (!texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            shaders_ = gl::create_shaders_program
            (
                R"(
                    uniform vec2 u_position;
                    uniform vec2 u_size;
                    uniform vec2 u_viewport;

                    attribute vec2 a_texture;

                    varying vec2 v_texture;

                    void main()
                    {
                        vec2 px_position = (a_texture * u_size + u_position) / u_viewport;
                        gl_Position = vec4((2.0 * px_position.x) - 1.0, 1.0 - (2.0 * px_position.y), 0.0, 1.0);
                        v_texture = a_texture;
                    }
                )"_glsl,
                R"(
                    precision mediump float;

                    uniform sampler2D s_texture;
                    varying vec2 v_texture;

                    void main()
                    {
                        gl_FragColor = texture2D(s_texture, v_texture);
                    }
                )"_glsl
            );

            if (!shaders_)
            {
                output_debug_string("create shaders error\n");
                return false;
            }

            u_position_ = u_position_.instance(shaders_, "u_position"_zsv);
            u_size_ = u_size_.instance(shaders_, "u_size"_zsv);
            u_viewport_ = u_viewport_.instance(shaders_, "u_viewport"_zsv);
            s_texture_ = s_texture_.instance(shaders_, "s_texture"_zsv);

            const vertext_type vertices[]
            {
                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 0.0f},
                {1.0f, 1.0f}
            };

            vbo_ = gl::vertex_attrib_buffer{ vertices, gl::get_attribute_locations(shaders_, "a_texture"_zsv) };
            if (!vbo_)
            {
                output_debug_string("create vertex buffer error\n");
                return false;
            }

            gl::use(shaders_);

            u_size_.store(vec2_cast<gl::vec2f_t>(texture_.sizes()));
            u_viewport_.store(vec2_cast<gl::vec2f_t>(egl_window_->viewport));

            return true;
        }

        void show() noexcept
        {
            need_redraw_ = true;
            ui::show(egl_window_, ui::show_command::show_maximazed);
        }

        void draw() const noexcept
        {
            if (const auto lock = egl::begin_painting(egl_window_))
            {
                gl::clear(colors::white_f);

                gl::use(shaders_);

                s_texture_.store(texture_);

                const auto vbo_user = vbo_.bind();

                const auto viewport = egl_window_->viewport;
                const auto dx = texture_.width() + 1u;
                const auto dy = texture_.height() + 1u;

                for (upixel_t x = 0; x < viewport.width(); x += dx)
                {
                    for (upixel_t y = 0; y < viewport.height(); y += dy)
                    {
                        u_position_.store(vec2_cast<gl::vec2f_t>(vec2{ x, y }));

                        vbo_user.draw(gl::draw_mode::triangle_strip);
                    }
                }
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
            return ui::run_event_loop(egl_window_, *this);
        }

    private:
        egl::window egl_window_;
        gl::texture_image2d texture_;
        gl::vertex_attrib_buffer<vertext_type> vbo_;
        gl::texture_sampler2D_t s_texture_ = gl::invalidtexturesampler;
        gl::shaders_program_t shaders_ = gl::nullprogram;
        gl::uniform_vec2f_t u_position_ = gl::invaliduniform;
        gl::uniform_vec2f_t u_size_ = gl::invaliduniform;
        gl::uniform_vec2f_t u_viewport_ = gl::invaliduniform;


        bool need_redraw_{ false };
    };
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    main_processor processor;

    if (!processor.initialize())
    {
        output_debug_string("initialize fail\n");
        return -1;
    }

    processor.show();

    return processor.run();
}




