#include <array>

#include <core/lerp.h>

#include <debug/debug.h>

#include <px/pixspan.h>

#include <egl_ui/egl_ui_owner.h>

#include <utility/shader_library.h>


namespace
{
    gl::texture2d lumpixmap_gallery_rendering() noexcept
    {
        static constexpr luminance_pixspace image_space{ 9_npx, 11_npx };

        static constexpr luminance_t image_data[image_space.size()]
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

        static constexpr auto w_image_space = image_space.width() + 1_npx;
        static constexpr auto h_image_space = image_space.height() + 1_npx;

        static constexpr luminance_pixspace gallery_space{ 4u * w_image_space, 3u * h_image_space };

        static constexpr auto gallery_data_gen = [] () noexcept
        {
            std::array<luminance_t, gallery_space.size()> gallery_data{};
            {
                constexpr pixspan image_span{ image_data, image_space };
                const pixspan gallery_span{ std::data(gallery_data), gallery_space };
                for (npx_t y = 0_npx; y < gallery_space.height(); y += h_image_space)
                {
                    for (npx_t x = 0_npx; x < gallery_space.width(); x += w_image_space)
                    {
                        gallery_span.store(x, y, image_span);
                    }
                }
            }
            return gallery_data;
        };

        static constexpr auto gallery_data = gallery_data_gen();
        static constexpr pixspan gallery_image{ std::data(gallery_data), gallery_space };

        return gl::create_texture2d(gallery_image);
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

    const auto texture = lumpixmap_gallery_rendering();
    if (!texture)
    {
        e_debug("pixmap rendering fail: {}", glGetError());
        return EXIT_FAILURE;
    }

    shader_library<vert::positioned_texture, frag::luminance_texture_mix_color>  shaders{};
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }

    shaders.use();
    shaders.vert().size(texture.sizes());
    shaders.vert().viewport(egl.viewport());
    shaders.frag().texture(texture);

    {
        const egl_painting_owner painting_owner{ egl };
        gl::viewport(egl.viewport());
        gl::clear(colors::white_f);

        const auto vb = shaders.vert().frame();

        const auto [w, h] = egl.viewport();
        const auto dx = texture.width() + 1_npx;
        const auto dy = texture.height() + 1_npx;

        const auto x_color_lerp = lerp(0_npx, w, colors::blue, colors::red);

        for (npx_t x = 0_npx; x < w; x += dx)
        {
            const auto xy_color_lerp = lerp(0_npx, h, to_color(x_color_lerp(x)), colors::green);

            for (npx_t y = 0_npx; y < h; y += dy)
            {
                shaders.frag().color(to_colorf(xy_color_lerp(y)));
                shaders.vert().position(x, y);
                vb.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




