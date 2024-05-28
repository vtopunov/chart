#include <debug/debug.h>

#include <px/algorithm.h>
#include <px/pixmap.h>

#include <egl_ui/egl_ui_owner.h>

#ifdef D_OS_WINDOWS
#include <file/file_mmap.h>
#endif

#include <utility/shader_library.h>

#include "test_figure.h"


namespace
{
    constexpr void draw_dda_line(const lumpixspan image, double x0, double y0, double x1, double y1) noexcept
    {
        const auto setpix = [data = image.data(), line_size = image.line_size()](size_t x, size_t y) noexcept
        {
            *(data + line_size * y + x) = luminance_max;
        };

        constexpr auto uz_round = [] (double v) noexcept
        {
            return static_cast<size_t>(v + 0.5);
        };

        constexpr auto abs_distance = [] (double v0, double v1) noexcept
        {
            return std::max(v0 - v1, v1 - v0);
        };

        const auto steps = uz_round(std::max(abs_distance(x0, x1), abs_distance(y0, y1)));

        const auto uz_lerp = [steps, uz_round] (uint64_t i, double v0, double v1) noexcept
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

    constexpr void draw_dda_line(const lumpixspan image, const vtest_line_figure::line& line) noexcept
    {
        draw_dda_line(image, line.x0, line.y0, line.x1, line.y1);
    }

    constexpr void draw_antialiasing_line(const lumpixspan image, const vtest_line_figure::line& line) noexcept
    {
        draw_antialiasing_line(image, line.x0, line.y0, line.x1, line.y1);
    }

    gl::texture2d lines_rendering() noexcept
    {
        using vtest_line_figure::figure;

        lumpixmap image{ 600_npx, 600_npx };
        if (!image)
        {
            e_debug("out of memory");
            return {};
        }

        {
            constexpr auto end_figure = std::cend(figure) - 2_z;
            for (auto it = std::cbegin(figure); it < end_figure; ++it )
            {
                draw_antialiasing_line(image, *it);
                draw_dda_line(image, *++it);
                draw_antialiasing_line(image, *++it);
            }
        }

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
    const auto egl = create_egl_ui(app);
    if (!egl)
    {
        e_debug("create window error: {}", egl_ui::error_code());
        return EXIT_FAILURE;
    }

    const auto texture = lines_rendering();
    if (!texture)
    {
        e_debug("lines rendering fail");
        return EXIT_FAILURE;
    }

    shader_library<vert::positioned_texture, frag::inverted_texture>  shaders{};
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }
    
    {
        const egl_painting_owner painting_owner{ egl };
        gl::viewport(egl.viewport());
        gl::clear(colors::white_f);

        shaders.use();
        shaders.frag().texture(texture);
        shaders.vert().viewport(egl.viewport());
        shaders.vert().position(30_npx, 50_npx);
        shaders.vert().size(texture.sizes());
        shaders.vert().frame().draw();
    }

    return ui::run_event_loop(egl);
}




