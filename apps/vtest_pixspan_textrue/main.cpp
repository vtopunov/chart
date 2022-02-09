#include <core/color.h>
#include <core/lerp.h>

#include <os/debug.h>

#include <ui/event_loop.h>

#include <egl/window.h>

#include <file/file_mmap.h>

#include <image/png.h>

#include <utility/shaders_library.h>


namespace
{
    gl::texture2d image_gallery_rendering() noexcept
    {
        gl::texture2d result_texture;

        image::pixrgba32map image;
        if (const auto errc = image::png_decode_image(file::mmap(_PATH("grid_9x9.png")), image); image::png_errno::OK != errc)
        {
            e_debug("png error {}:{}\n", to_underlying(errc), image::png_error_string(errc).c_str());
            return result_texture;
        }

        constexpr auto sep = 1_px;
        const auto w_image_space = image.width() + sep;
        const auto h_image_space = image.height() + sep;

        image::pixrgba32map gallery{ 3u * w_image_space + sep, 3u * h_image_space + sep };
        if (!gallery)
        {
            e_debug("out of memory\n");
            return result_texture;
        }

        for (pxside_t y = sep; y < gallery.height(); y += h_image_space)
        {
            for (pxside_t x = sep; x < gallery.width(); x += w_image_space)
            {
                gallery.store(x, y, image);
            }
        }

        result_texture = gl::create_texture2d(gallery);
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

    if (const auto lock = begin_painting(egl))
    {
        gl::clear(gl::colors::blue_f);

        const auto vb = shaders.vert.a_frame.bind();

        const auto surface_sizes = sizes(egl);
        const auto dx = width(texture) + 2_px;
        const auto dy = height(texture) + 2_px;

        for (pxside_t y = 0; y < surface_sizes.height(); y += dy)
        {
            for (pxside_t x = 0; x < surface_sizes.width(); x += dx)
            {
                shaders.vert.u_position.store(point2d{ x, y });
                vb.draw();
            }
        }
    }

    ui::show(egl, ui::show_command::show_maximazed);

    return ui::run_event_loop(egl);
}




