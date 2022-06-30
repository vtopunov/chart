#include <core/lerp.h>

#include <debug/debug.h>

#include <px/pixspan.h>

#include <egl/event_loop.h>

#include <utility/shaders_library.h>

namespace
{
    gl::texture2d pix8map_gallery_rendering() noexcept
    {
        gl::texture2d result_texture;

        using pix8_t = pix8span::pixel_type;

        constexpr pix8space image_sizes{ 9_px, 11_px };

        static constexpr pix8_t image[image_sizes.size()]
        {
            0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

            0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x66, 0x33, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x66, 0x33, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x66, 0x33, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

            0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00
        };

        constexpr const_pix8span image_span{ image, image_sizes };

        constexpr auto w_image_space = image_span.width() + 1_px;
        constexpr auto h_image_space = image_span.height() + 1_px;

        constexpr pix8space gallery_sizes{ 4u * w_image_space, 3u * h_image_space };
        pix8_t gallery[gallery_sizes.size()]{};
        const pix8span gallery_span{ gallery, gallery_sizes };

        for (pxside_t y = 0_px; y < gallery_sizes.height(); y += h_image_space)
        {
            for (pxside_t x = 0_px; x < gallery_sizes.width(); x += w_image_space)
            {
                gallery_span.store(x, y, image_span);
            }
        }

        result_texture = gl::create_texture2d(gallery_span);
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

    const auto texture = pix8map_gallery_rendering();
    if (!texture)
    {
        e_debug("pixmap rendering fail");
        return EXIT_FAILURE;
    }

    shaders_library<vert::positioned_texture, frag::gray_texture_mix_color>  shaders;
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }

    shaders.use();
    shaders.vert.u_size.store(sizes(texture));
    shaders.vert.u_viewport.store(sizes(egl));
    shaders.frag.s_texture.store(texture);

    {
        egl::painting_owner painting_lock{ egl };
        gl::clear(gl::colors::white_f);

        const auto vb = shaders.vert.a_frame.bind();

        const auto [w, h] = sizes(egl);
        const auto dx = width(texture) + 1_px;
        const auto dy = height(texture) + 1_px;

        const auto x_color_lerp = lerp(num_range{ 0_px, w }, num_range{ colors::blue, colors::red });

        for (pxside_t x = 0_px; x < w; x += dx)
        {
            const auto xy_color_lerp = lerp(num_range{ 0_px, h }, num_range{ color_cast<rgba_color32_t>(x_color_lerp(x)), colors::green });

            for (pxside_t y = 0_px; y < h; y += dy)
            {
                shaders.frag.u_color.store(gl::to_colorf(xy_color_lerp(y)));
                shaders.vert.u_position.store(point2d{ x, y });
                vb.draw();
            }
        }
    }

    return run(egl);
}




