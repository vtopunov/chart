#include <core/color.h>
#include <core/lerp.h>
#include <core/debug.h>

#include <ui/event_loop.h>
#include <egl/window.h>

#include <apps/utility/shaders.h>

using namespace std::string_view_literals;
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

            constexpr px::size2d_t image_sizes{ 9_px, 9_px };

            constexpr GLubyte image[image_sizes.height() * size_align<4>(image_sizes.width())]
            {
                0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,
                0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
                0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

                0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
                0xff, 0xcc, 0x99,  0x66, 0x33, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
                0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

                0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
                0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
                0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00
            };

            texture_ = gl::create_texture2d(image_sizes, image);
            if (!texture_)
            {
                output_debug_string("create texture error: {}\n", glGetError());
                return false;
            }

            if (!shaders_.build())
            {
                output_debug_string("build shaders program error\n");
                return false;
            }

            shaders_.use();

            shaders_.frag.u_color.store(colors::black_f);
            shaders_.vert.u_position.store(fill_vec2(0.f));
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

                const auto [w, h] = sizes(egl_);
                const auto dx = width(texture_) + 1_px;
                const auto dy = height(texture_) + 1_px;

                const auto x_color_lerp = lerp(num_range{ 0_px, w }, num_range{ colors::blue, colors::red });

                for (pxside_t x = 0_px; x < w; x += dx)
                {
                    const auto xy_color_lerp = lerp(num_range{ 0_px, h }, num_range{ color_cast<rgba_color32_t>(x_color_lerp(x)), colors::green });

                    for (pxside_t y = 0_px; y < h; y += dy)
                    {
                        shaders_.frag.u_color.store(color_cast<rgba_colorf_t>(xy_color_lerp(y)));
                        shaders_.vert.u_position.store(narrow2d_cast<gl::vec2f_t>(x, y));
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
        shaders<vert::positioned_texture, frag::gray_texture_mix_color>  shaders_;

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




