#include <core/lerp.h>

#include <debug/debug.h>

#include <egl_ui/egl_ui_owner.h>

#include <file/file_asset.h>

#include <image/png.h>

#include <shader/library.h>


namespace
{
    gl::texture2d image_gallery_rendering() noexcept
    {
        const auto asset = file::asset::mmap(_PATH("grid_9x9.png"));
        if (!asset)
        {
            e_debug("can't open file");
            return {};
        }

        const auto png = image::png_decode_to_rgba(asset);
        if (!png)
        {
            const auto errc = png.error_code();
            e_debug("png error {}:{}", to_underlying(errc), image::png_error_string(errc).c_str());
            return {};
        }

        constexpr auto sep = 1_npx;
        const auto w_image_space = png.width() + sep;
        const auto h_image_space = png.height() + sep;

        pixmap<rgba_color> gallery{ 3u * w_image_space + sep, 3u * h_image_space + sep };
        if (!gallery)
        {
            e_debug("out of memory");
            return {};
        }

        for (npx_t y = sep; y < gallery.height(); y += h_image_space)
        {
            for (npx_t x = sep; x < gallery.width(); x += w_image_space)
            {
                gallery.store(x, y, png);
            }
        }

        auto result_texture = gl::create_texture2d(view(gallery));
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
    const auto egl = create_egl_ui(app);
    if (!egl)
    {
        e_debug("create window error: {}", egl_ui::error_code());
        return EXIT_FAILURE;
    }

    const auto texture = image_gallery_rendering();
    if (!texture)
    {
        e_debug("text rendering fail");
        return EXIT_FAILURE;
    }

    shader_embed::default_texture shaders{};
    if (!shaders.load())
    {
        e_debug("build shaders program error");
        return false;
    }

    shaders.viewport(egl.viewport());
    shaders.texture(texture);

    {
        const egl_painting_owner painting_owner{ egl };
        gl::viewport(egl.viewport());
        gl::clear(colors::white_f);

        const auto [w, h] = egl.viewport();
        const auto dx = texture.width() + 1_npx;
        const auto dy = texture.height() + 1_npx;

        for (npx_t y = 0; y < h; y += dy)
        {
            for (npx_t x = 0; x < w; x += dx)
            {
                shaders.position(x, y);
                shaders.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




