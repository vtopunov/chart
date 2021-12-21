#include <array>

#include <core/color.h>
#include <core/lerp.h>
#include <core/debug.h>

#include <ui/event_loop.h>

#include <egl/window.h>

#include <file/file_mmap.h>

#include <font/font.h>

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
                output_debug_string
                (
                    "Instance of window is not high dpi. " 
                    "Add <dpiAware>true</dpiAware> in manifest. " 
                    "Resolution: {}x{}\n", resolution.width(), resolution.height()
                );
                return false;
            }

            pix8map_t image{ 300_px, 60_px };
            if (!image)
            {
                output_debug_string("out of memory\n");
                return false;
            }

            const auto font_file = file::mmap(_PATH("fonts/DroidSerif-Regular.ttf"));
            if (!font_file)
            {
                output_debug_string("can't open font file\n");
                return false;
            }

            const auto face = font::create_font(font_file, 20_px);
            if (!face)
            {
                output_debug_string("can't create font\n");
                return false;
            }

            draw_text(image, 3_px, 20_px, face, u8"Привет мир !_!`"sv);

            size(face, 15_px);

            {
                constexpr auto text = u8"Правый верх"sv;
                const auto tm = metrics(face, text);
                draw_text(image, image.width()-tm.width, -tm.top, face, text);
            }

            {
                constexpr auto text = u8">>> Центр <<<"sv;

                const auto tm = metrics(face, text);

                draw_text
                (
                    image,
                    (image.width() - tm.width ) / 2u,
                    (image.height()  + tm.bottom - tm.top ) / 2u,
                    face,
                    text
                );
            }

            {
                constexpr auto text1 = u8"____"sv;
                constexpr auto text2 = u8"````"sv;
                constexpr auto text3 = u8"Правый низ"sv;

                const auto tm = metrics(face, text3, metrics(face, text2, metrics(face, text1)));
                if (!tm)
                {
                    output_debug_string("invalid text metrics\n");
                    return false;
                }

                auto cursor = draw_text
                (
                    image,
                    image.width() - tm.width,
                    image.height() - tm.bottom,
                    face,
                    text1
                );

                cursor = draw_text(image, cursor, face, text2);
                cursor = draw_text(image, cursor, face, text3);
            }

            texture_ = gl::create_texture2d(image);
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
            shaders_.vert.u_position.store(0.f, 0.f);
            shaders_.vert.u_size.store(narrow2d_cast<gl::vec2f_t>(sizes(texture_)));
            shaders_.vert.u_viewport.store(narrow2d_cast<gl::vec2f_t>(sizes(egl_)));

            return true;
        }

        void show() noexcept
        {
            need_redraw_ = true;
            ui::show(egl_, ui::show_command::show_maximazed);
        }

        void draw() const noexcept
        {
            if (const auto lock = begin_painting(egl_))
            {
                gl::clear(colors::red_f);

                shaders_.use();
                shaders_.frag.s_texture.store(texture_);
                const auto vbo_user = shaders_.vert.vbo.bind();

                const auto [w, h] = sizes(egl_);
                const auto dx = width(texture_) + 1_px;
                const auto dy = height(texture_) + 1_px;

                const auto y_color_lerp = lerp(num_range{ 0_px, h }, num_range{ colors::red, colors::blue });

                for (pxside_t y = 0; y < h; y += dy)
                {
                    const auto yx_color_lerp = lerp(num_range{ 0_px, w }, num_range{ colors::green, color_cast<rgba_color32_t>(y_color_lerp(y)) });

                    for (pxside_t x = 0; x < w; x += dx)
                    {
                        shaders_.frag.u_color.store(color_cast<rgba_colorf_t>(yx_color_lerp(x)));
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




