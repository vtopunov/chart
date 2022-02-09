#include <core/color.h>

#include <os/debug.h>

#include <px/pixalgorithm.h>

#include <ui/event_loop.h>

#include <egl/window.h>
#include <utility/shaders_library.h>

namespace
{
    constexpr void draw_dda_line(const pix8line line, double x0, double y0, double x1, double y1) noexcept
    {
        const auto setpix = [line](size_t x, size_t y) noexcept
        {
            *(line.position + line.size * y + x)  = numeric_max_v<u8tint_t>;
        };

        constexpr auto uz_round = [](double v) noexcept
        {
            return static_cast<size_t>(v + 0.5);
        };

        constexpr auto abs_distance = [](double v0, double v1) noexcept
        {
            return std::max(v0 - v1, v1 - v0);
        };

        const auto steps = uz_round(std::max(abs_distance(x0, x1), abs_distance(y0, y1)));

        const auto uz_lerp = [steps, uz_round](uint64_t i, double v0, double v1) noexcept
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

    gl::texture2d lines_rendering() noexcept
    {
        gl::texture2d result_texture;
    
        pix8map image{ 600_px, 600_px };
        if (!image)
        {
            e_debug("out of memory");
            return result_texture;
        }

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

        result_texture = gl::create_texture2d(image);
        if (!result_texture)
        {
            e_debug("create texture error: {}\n", glGetError());
            return {};
        }

        return result_texture;
    }
}


int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    const auto egl = egl::window_factory{}.create();
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    if (!is_maximum_resolution(egl))
    {
        e_debug("Instance of window is not high dpi. Add <dpiAware>true</dpiAware> in manifest");
        return EXIT_FAILURE;
    }

    const auto texture = lines_rendering();
    if (!texture)
    {
        e_debug("lines rendering fail");
        return EXIT_FAILURE;
    }

    shaders_library<vert::positioned_texture, frag::gray_texture_mix_color>  shaders;
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }

    shaders.use();
    shaders.vert.u_position.store(point2d{ 30_px, 50_px });
    shaders.vert.u_size.store(sizes(texture));
    shaders.vert.u_viewport.store(sizes(egl));
    shaders.frag.u_color.store(gl::colors::black_f);
    shaders.frag.s_texture.store(texture);
    
    if (const auto lock = begin_painting(egl))
    {
        gl::clear(gl::colors::gray_f);
        shaders.vert.a_frame.draw();
    }

    ui::show(egl, ui::show_command::show_maximazed);

    return ui::run_event_loop(egl);
}




