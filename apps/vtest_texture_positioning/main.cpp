#include <core/color.h>
#include <core/debug.h>

#include <ui/event_loop.h>
#include <egl/window.h>

#include <apps/utility/png.h>
#include <apps/utility/shaders.h>

using namespace std::string_view_literals;
using namespace std::chrono_literals;
using namespace gl_literals;

namespace
{
    class main_processor
    {
    public:
        constexpr main_processor() noexcept = default;

        D_DISABLE_COPY_MOVE(main_processor);

        bool initialize() noexcept
        {
            egl_ = egl::window_factory{}.create();
            if (!egl_)
            {
                output_debug_string("create window error: window error: {}, egl error: {}\n",
                    ui::error_code(), eglGetError());
                return false;
            }

            if (const auto resolution = ui::display_resolution(); sizes(egl_) != resolution)
            {
                output_debug_string("Instance of window is not high dpi. Add <dpiAware>true</dpiAware> in manifest. Resolution: {}x{}", resolution.width(), resolution.height());
                return false;
            }

            texture_ = png_reader{}.texture_from_file(_PATH("grid_9x9.png"));
            if (!texture_)
            {
                output_debug_string("create png texture error: {}\n", glGetError());
                return false;
            }

            if (!shaders_.build())
            {
                output_debug_string("build shaders program error\n");
                return false;
            }


            shaders_.use();
            shaders_.vert.u_position.store(0.f, 0.f);
            shaders_.vert.u_size.store(narrow2d_cast<gl::vec2f_t>(sizes(texture_)));
            shaders_.vert.u_viewport.store(narrow2d_cast<gl::vec2f_t>(sizes(egl_)));

            return true;
        }

        void show() noexcept
        {
            ui::show(egl_, ui::show_command::show_maximazed);
        }

        void draw() const noexcept
        {
            if (const auto lock = egl::begin_painting(egl_))
            {
                gl::clear(colors::white_f);
                
                shaders_.use();
                shaders_.frag.s_texture.store(texture_);
                const auto vbo_user = shaders_.vert.vbo.bind();

                const auto viewport = sizes(egl_);
                const auto dx = width(texture_) + 1_px;
                const auto dy = height(texture_) + 1_px;

                for (pxside_t y = 0_px; y < viewport.height(); y += dy)
                {
                    const auto yf = narrow_cast<GLfloat>(y);

                    for (pxside_t x = 0_px; x < viewport.width(); x += dx)
                    {
                        const auto xf = narrow_cast<GLfloat>(x);

                        shaders_.vert.u_position.store(xf, yf);

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
            return ui::run_event_loop(egl_, *this);
        }

    private:
        egl::window_t egl_;
        gl::texture2d_t texture_;
        shaders<vert::positioned_texture, frag::default_texture> shaders_;

        bool need_redraw_{ true };
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




