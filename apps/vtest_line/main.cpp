#include <core/color.h>
#include <core/debug.h>

#include <px/pixalgorithm.h>

#include <ui/event_loop.h>

#include <egl/window.h>
#include <apps/utility/shaders.h>

namespace
{
    constexpr void draw_dda_line(const pix8line_t line, double_t x0, double_t y0, double_t x1, double_t y1) noexcept
    {
        const auto setpix = [line](size_t x, size_t y) noexcept
        {
            *(line.position + line.size * y + x)  = numeric_max_v<u8tint_t>;
        };

        constexpr auto uz_round = [](double_t v) noexcept
        {
            return static_cast<size_t>(v + 0.5);
        };

        constexpr auto abs_distance = [](double_t v0, double_t v1) noexcept
        {
            return std::max(v0 - v1, v1 - v0);
        };

        const auto steps = uz_round(std::max(abs_distance(x0, x1), abs_distance(y0, y1)));

        const auto uz_lerp = [steps, uz_round](uint64_t i, double_t v0, double_t v1) noexcept
        {
            return uz_round(((steps - i) * v0 + i * v1) / steps);
        };

        for (uint64_t i = 0u; i <= steps; ++i)
        {
            const auto x = uz_lerp(i, x0, x1);
            const auto y = uz_lerp(i, y0, y1);
            setpix(x, y);
        }
    }

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

            pix8map_t image{ 600_px, 600_px };

            const auto line0 = image.line0();

            draw_antialiasing_line(line0, 240, 315, 5, 325); // 180+
            draw_dda_line(line0, 240, 320, 5, 330);
            draw_antialiasing_line(line0, 5, 335, 240, 325);

            draw_antialiasing_line(line0, 240, 295, 5, 295); // 180
            draw_dda_line(line0, 240, 300, 5, 300); 
            draw_antialiasing_line(line0, 5, 305, 240, 305);

            draw_antialiasing_line(line0, 240, 275, 5, 265); // 180-
            draw_dda_line(line0, 240, 280, 5, 270);
            draw_antialiasing_line(line0, 5, 275, 240, 285);

            draw_antialiasing_line(line0, 240, 255, 5, 40);  // 135+
            draw_dda_line(line0, 240, 260, 5, 45);
            draw_antialiasing_line(line0, 5, 50, 240, 265);

            draw_antialiasing_line(line0, 245, 240, 10, 5);  // 135
            draw_dda_line(line0, 240, 240, 5, 5);
            draw_antialiasing_line(line0, 5, 10, 240, 245);

            draw_antialiasing_line(line0, 265, 240, 50, 5);  // 135-
            draw_dda_line(line0, 260, 240, 45, 5);
            draw_antialiasing_line(line0, 40, 5, 255, 240);

            draw_antialiasing_line(line0, 275, 240, 265, 5);  // 90+
            draw_dda_line(line0, 280, 240, 270, 5);
            draw_antialiasing_line(line0, 275, 5, 285, 240);

            draw_antialiasing_line(line0, 295, 240, 295, 5);  // 90
            draw_dda_line(line0, 300, 240, 300, 5);
            draw_antialiasing_line(line0, 305, 5, 305, 240);

            draw_antialiasing_line(line0, 315, 240, 325, 5);  // 90-
            draw_dda_line(line0, 320, 240, 330, 5);
            draw_antialiasing_line(line0, 335, 5, 325, 240);

            draw_antialiasing_line(line0, 335, 240, 550, 5);  // 45+
            draw_dda_line(line0, 340, 240, 555, 5);
            draw_antialiasing_line(line0, 560, 5, 345, 240);

            draw_antialiasing_line(line0, 355, 240, 590, 5);  // 45
            draw_dda_line(line0, 360, 240, 595, 5);
            draw_antialiasing_line(line0, 595, 10, 360, 245);

            draw_antialiasing_line(line0, 360, 255, 595, 40);  // 45-
            draw_dda_line(line0, 360, 260, 595, 45);
            draw_antialiasing_line(line0, 595, 50, 360, 265);

            draw_antialiasing_line(line0, 360, 275, 590, 265); // 0+
            draw_dda_line(line0, 360, 280, 590, 270);
            draw_antialiasing_line(line0, 590, 275, 360, 285);

            draw_antialiasing_line(line0, 360, 295, 590, 295); // 0
            draw_dda_line(line0, 360, 300, 590, 300);
            draw_antialiasing_line(line0, 590, 305, 360, 305);

            draw_antialiasing_line(line0, 360, 315, 590, 325); // 0-
            draw_dda_line(line0, 360, 320, 590, 330);
            draw_antialiasing_line(line0, 590, 335, 360, 325);

            draw_antialiasing_line(line0, 360, 335, 595, 550);  // -45+
            draw_dda_line(line0, 360, 340, 595, 555);
            draw_antialiasing_line(line0, 595, 560, 360, 345);

            draw_antialiasing_line(line0, 360, 355, 595, 590);  // -45
            draw_dda_line(line0, 360, 360, 595, 595); 
            draw_antialiasing_line(line0, 590, 595, 355, 360);

            draw_antialiasing_line(line0, 345, 360, 560, 595);  // -45-
            draw_dda_line(line0, 340, 360, 555, 595);
            draw_antialiasing_line(line0, 550, 595, 335, 360);

            draw_antialiasing_line(line0, 325, 360, 335, 595); // -90+
            draw_dda_line(line0, 320, 360, 330, 595);
            draw_antialiasing_line(line0, 325, 595, 315, 360);

            draw_antialiasing_line(line0, 305, 360, 305, 595); // -90
            draw_dda_line(line0, 300, 360, 300, 595);
            draw_antialiasing_line(line0, 295, 595, 295, 360);

            draw_antialiasing_line(line0, 285, 360, 275, 595); // -90-
            draw_dda_line(line0, 280, 360, 270, 595);
            draw_antialiasing_line(line0, 265, 595, 275, 360);

            draw_antialiasing_line(line0, 265, 360, 50, 595); // -135+
            draw_dda_line(line0, 260, 360, 45, 595);
            draw_antialiasing_line(line0, 40, 595, 255, 360);

            draw_antialiasing_line(line0, 245, 360, 10, 595); // -135
            draw_dda_line(line0, 240, 360, 5, 595);
            draw_antialiasing_line(line0, 5, 590, 240, 355);

            draw_antialiasing_line(line0, 240, 345, 5, 560); // -135-
            draw_dda_line(line0, 240, 340, 5, 555);
            draw_antialiasing_line(line0, 5, 550, 240, 335);

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
            shaders_.vert.u_position.store(30.f, 50.f);
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
            if (const auto lock = begin_painting(egl_))
            {
                gl::clear(colors::gray_f);

                shaders_.use();
                shaders_.frag.s_texture.store(texture_);
                shaders_.vert.vbo.draw(gl::draw_mode::triangle_strip);
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


int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
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




