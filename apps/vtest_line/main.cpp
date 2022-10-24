#include <debug/debug.h>

#include <px/algorithm.h>

#include <egl_ui/run.h>

#include <file/file_mmap.h>

#include <utility/shaders_library.h>

namespace
{
    constexpr void draw_dda_line(const pix8span image, double x0, double y0, double x1, double y1) noexcept
    {
        const auto setpix = [data = image.data(), line_size = image.line_size()](size_t x, size_t y) noexcept
        {
            *(data + line_size * y + x)  = numeric_max_v<u8tint_t>;
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
        pix8map image{ 600_px, 600_px };
        if (!image)
        {
            e_debug("out of memory");
            return {};
        }


        draw_antialiasing_line(image, 240, 315, 5, 325); // 180+
        draw_dda_line(image, 240, 320, 5, 330);
        draw_antialiasing_line(image, 5, 335, 240, 325);

        draw_antialiasing_line(image, 240, 295, 5, 295); // 180
        draw_dda_line(image, 240, 300, 5, 300);
        draw_antialiasing_line(image, 5, 305, 240, 305);

        draw_antialiasing_line(image, 240, 275, 5, 265); // 180-
        draw_dda_line(image, 240, 280, 5, 270);
        draw_antialiasing_line(image, 5, 275, 240, 285);

        draw_antialiasing_line(image, 240, 255, 5, 40);  // 135+
        draw_dda_line(image, 240, 260, 5, 45);
        draw_antialiasing_line(image, 5, 50, 240, 265);

        draw_antialiasing_line(image, 245, 240, 10, 5);  // 135
        draw_dda_line(image, 240, 240, 5, 5);
        draw_antialiasing_line(image, 5, 10, 240, 245);

        draw_antialiasing_line(image, 265, 240, 50, 5);  // 135-
        draw_dda_line(image, 260, 240, 45, 5);
        draw_antialiasing_line(image, 40, 5, 255, 240);

        draw_antialiasing_line(image, 275, 240, 265, 5);  // 90+
        draw_dda_line(image, 280, 240, 270, 5);
        draw_antialiasing_line(image, 275, 5, 285, 240);

        draw_antialiasing_line(image, 295, 240, 295, 5);  // 90
        draw_dda_line(image, 300, 240, 300, 5);
        draw_antialiasing_line(image, 305, 5, 305, 240);

        draw_antialiasing_line(image, 315, 240, 325, 5);  // 90-
        draw_dda_line(image, 320, 240, 330, 5);
        draw_antialiasing_line(image, 335, 5, 325, 240);

        draw_antialiasing_line(image, 335, 240, 550, 5);  // 45+
        draw_dda_line(image, 340, 240, 555, 5);
        draw_antialiasing_line(image, 560, 5, 345, 240);

        draw_antialiasing_line(image, 355, 240, 590, 5);  // 45
        draw_dda_line(image, 360, 240, 595, 5);
        draw_antialiasing_line(image, 595, 10, 360, 245);

        draw_antialiasing_line(image, 360, 255, 595, 40);  // 45-
        draw_dda_line(image, 360, 260, 595, 45);
        draw_antialiasing_line(image, 595, 50, 360, 265);

        draw_antialiasing_line(image, 360, 275, 590, 265); // 0+
        draw_dda_line(image, 360, 280, 590, 270);
        draw_antialiasing_line(image, 590, 275, 360, 285);

        draw_antialiasing_line(image, 360, 295, 590, 295); // 0
        draw_dda_line(image, 360, 300, 590, 300);
        draw_antialiasing_line(image, 590, 305, 360, 305);

        draw_antialiasing_line(image, 360, 315, 590, 325); // 0-
        draw_dda_line(image, 360, 320, 590, 330);
        draw_antialiasing_line(image, 590, 335, 360, 325);

        draw_antialiasing_line(image, 360, 335, 595, 550);  // -45+
        draw_dda_line(image, 360, 340, 595, 555);
        draw_antialiasing_line(image, 595, 560, 360, 345);

        draw_antialiasing_line(image, 360, 355, 595, 590);  // -45
        draw_dda_line(image, 360, 360, 595, 595);
        draw_antialiasing_line(image, 590, 595, 355, 360);

        draw_antialiasing_line(image, 345, 360, 560, 595);  // -45-
        draw_dda_line(image, 340, 360, 555, 595);
        draw_antialiasing_line(image, 550, 595, 335, 360);

        draw_antialiasing_line(image, 325, 360, 335, 595); // -90+
        draw_dda_line(image, 320, 360, 330, 595);
        draw_antialiasing_line(image, 325, 595, 315, 360);

        draw_antialiasing_line(image, 305, 360, 305, 595); // -90
        draw_dda_line(image, 300, 360, 300, 595);
        draw_antialiasing_line(image, 295, 595, 295, 360);

        draw_antialiasing_line(image, 285, 360, 275, 595); // -90-
        draw_dda_line(image, 280, 360, 270, 595);
        draw_antialiasing_line(image, 265, 595, 275, 360);

        draw_antialiasing_line(image, 265, 360, 50, 595); // -135+
        draw_dda_line(image, 260, 360, 45, 595);
        draw_antialiasing_line(image, 40, 595, 255, 360);

        draw_antialiasing_line(image, 245, 360, 10, 595); // -135
        draw_dda_line(image, 240, 360, 5, 595);
        draw_antialiasing_line(image, 5, 590, 240, 355);

        draw_antialiasing_line(image, 240, 345, 5, 560); // -135-
        draw_dda_line(image, 240, 340, 5, 555);
        draw_antialiasing_line(image, 5, 550, 240, 335);

#ifdef D_OS_WINDOWS
        {
            const auto test_image = file::mmap(_PATH("test_blob.bin"));
            D_ASSERT(test_image.r().size() == image.size());
            D_ASSERT(!memcmp(test_image.r().data(), image.data(), image.size()));
        }
#endif

        auto result_texture = gl::create_texture2d(image);
        if (!result_texture)
        {
            e_debug("create texture error: {}\n", glGetError());
            return {};
        }

        return result_texture;
    }
}


int app_main(os::module_handle_t app) noexcept
{
    const auto egl = create_egl_window(app);
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    const auto texture = lines_rendering();
    if (!texture)
    {
        e_debug("lines rendering fail");
        return EXIT_FAILURE;
    }

    shaders_library<vert::positioned_texture, frag::inverted_texture>  shaders{};
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }
    
    {
        const egl_painting_owner painting_lock{ egl };
        gl::clear(gl::colors::gray_f);

        shaders.use();
        shaders.frag.s_texture.store(texture);
        shaders.vert.u_viewport.store(sizes(egl));
        shaders.vert.u_position.store(30_px, 50_px);
        shaders.vert.u_size.store(sizes(texture));
        shaders.vert.a_frame.draw();
    }

    return run(egl);
}




