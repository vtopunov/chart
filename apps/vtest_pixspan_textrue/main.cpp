#include <core/color.h>
#include <core/lerp.h>
#include <core/debug.h>

#include <ui/event_loop.h>

#include <egl/window.h>

#include <file/file_mmap.h>

#include <image/png.h>

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

            image::rgba32_pixmap_t image;
            if (const auto errc = image::png_decode_image(file::mmap(_PATH("grid_9x9.png")), image); image::png_errno::OK != errc)
            {
                output_debug_string("png error {}:{}\n", to_underlying(errc), image::png_error_string(errc).c_str());
                return false;
            }

            constexpr auto sep = 1_px;
            const auto w_image_space = image.width() + sep;
            const auto h_image_space = image.height() + sep;

            image::rgba32_pixmap_t gallery{ 3u * w_image_space + sep, 3u * h_image_space + sep };
            if (!gallery)
            {
                output_debug_string("out of memory\n");
                return false;
            }

            for (pxside_t y = sep; y < gallery.height(); y += h_image_space)
            {
                for (pxside_t x = sep; x < gallery.width(); x += w_image_space)
                {
                    gallery.store(x, y, image);
                }
            }

            texture_ = gl::create_texture2d(gallery);
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
                gl::clear(colors::blue_f);

                shaders_.use();
                shaders_.frag.s_texture.store(texture_);
                const auto vbo_user = shaders_.vert.vbo.bind();

                const auto surface_sizes = sizes(egl_);
                const auto dx = width(texture_) + 2_px;
                const auto dy = height(texture_) + 2_px;

                for (pxside_t y = 0; y < surface_sizes.height(); y += dy)
                {
                    for (pxside_t x = 0; x < surface_sizes.width(); x += dx)
                    {
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
        shaders<vert::positioned_texture, frag::default_texture>  shaders_;

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




