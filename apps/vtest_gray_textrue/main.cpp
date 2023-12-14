#include <core/lerp.h>

#include <debug/debug.h>

#include <egl_ui/egl_ui_owner.h>

#include <utility/shader_library.h>


namespace
{
    gl::texture2d pix8map_rendering() noexcept
    {
        constexpr pxsize2d image_sizes{ 9_px, 9_px };

        constexpr GLubyte image[image_sizes.height() * size_align<4>(image_sizes.width())]
        {
            0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

            0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x66, 0x33, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

            0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
            0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00
        };

        return gl::create_texture2d(image_sizes, image);
    }
}

int app_main(os::module_handle_t app) noexcept
{
    const auto egl = create_egl_ui(app);
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    const auto texture = pix8map_rendering();
    if (!texture)
    {
        e_debug("pixmap rendering fail: {}", glGetError());
        return EXIT_FAILURE;
    }

    shader_library<vert::positioned_texture, frag::gray_texture_mix_color> shaders{};
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }

    shaders.use();
    shaders.vert.u_size.store(sizes(texture));
    shaders.vert.u_viewport.store(egl.viewport);
    shaders.frag.s_texture.store(texture);

    {
        const egl_painting_owner painting_owner{ egl };
        gl::viewport(egl.viewport);
        gl::clear(gl::colors::white_f);

        const auto vb = shaders.vert.a_frame.bind();

        const auto [w, h] = egl.viewport;
        const auto dx = width(texture) + 1_px;
        const auto dy = height(texture) + 1_px;

        const auto x_color_lerp = lerp(0_px, w, colors::blue, colors::red);

        for (pxside_t x = 0_px; x < w; x += dx)
        {
            const auto xy_color_lerp = lerp(0_px, h, color_cast<rgba_color32_t>(x_color_lerp(x)), colors::green);

            for (pxside_t y = 0_px; y < h; y += dy)
            {
                shaders.frag.u_color.store(color_cast<gl::rgba_colorf_t>(xy_color_lerp(y)));
                shaders.vert.u_position.store(point2d{ x, y });
                vb.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




