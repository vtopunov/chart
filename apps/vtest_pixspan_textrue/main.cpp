#include <core/lerp.h>

#include <debug/debug.h>

#include <egl/event_loop.h>

#include <file/file_mmap.h>

#include <image/png.h>

#include <utility/shaders_library.h>


namespace
{
    gl::texture2d image_gallery_rendering() noexcept
    {
        const auto png = image::png_decode_to_r8g8b8a8(file::mmap(_PATH("grid_9x9.png")));
        if (!png)
        {
            const auto errc = png.error_code();
            e_debug("png error {}:{}", to_underlying(errc), image::png_error_string(errc).c_str());
            return {};
        }

        constexpr auto sep = 1_px;
        const auto w_image_space = png.width() + sep;
        const auto h_image_space = png.height() + sep;

        pixmap<rgba_color32_t> gallery{ 3u * w_image_space + sep, 3u * h_image_space + sep };
        if (!gallery)
        {
            e_debug("out of memory");
            return {};
        }

        for (pxside_t y = sep; y < gallery.height(); y += h_image_space)
        {
            for (pxside_t x = sep; x < gallery.width(); x += w_image_space)
            {
                gallery.store(x, y, png);
            }
        }

        auto result_texture = gl::create_texture2d(gallery);
        if (!result_texture)
        {
            e_debug("create texture error: {}", glGetError());
            return {};
        }

        return result_texture;
    }
}

int app_main(os::module_handle_t app) noexcept
{
    const auto egl = egl::instance(app);
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    const auto texture = image_gallery_rendering();
    if (!texture)
    {
        e_debug("text rendering fail");
        return EXIT_FAILURE;
    }

    shaders_library<vert::positioned_texture, frag::default_texture>  shaders{};
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return false;
    }

    shaders.use();
    shaders.vert.u_size.store(sizes(texture));
    shaders.vert.u_viewport.store(sizes(egl));
    shaders.frag.s_texture.store(texture);

    {
        egl::painting_owner painting_lock{ egl };
        gl::clear(gl::colors::white_f);

        const auto vb = shaders.vert.a_frame.bind();

        const auto surface_sizes = sizes(egl);
        const auto dx = width(texture) + 1_px;
        const auto dy = height(texture) + 1_px;

        for (pxside_t y = 0; y < surface_sizes.height(); y += dy)
        {
            for (pxside_t x = 0; x < surface_sizes.width(); x += dx)
            {
                shaders.vert.u_position.store(point2d{ x, y });
                vb.draw();
            }
        }
    }

    return run(egl);
}




