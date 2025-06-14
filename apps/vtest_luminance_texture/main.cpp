#include <core/lerp.h>

#include <debug/debug.h>

#include <egli/egli_owner.h>

#include <shader/library.h>


namespace
{
    gl::texture2d lumpixmap_rendering() noexcept
    {
        static constexpr luminance_pixspace image_space{ 9_npx, 9_npx };

        static constexpr luminance_t image_data[image_space.size_bytes()]
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

        static constexpr pixspan image{ image_data, image_space };
        
        return gl::create_texture2d(image);
    }
}

int main() noexcept
{
    const auto egl = egli_builder{}.build();
    if (!egl)
    {
        e_debug("create window error: {}", egli::error_code());
        return EXIT_FAILURE;
    }

    const auto texture = lumpixmap_rendering();
    if (!texture)
    {
        e_debug("pixmap rendering fail: {}", glGetError());
        return EXIT_FAILURE;
    }

    shader_embed::luminance_texture shaders{};
    if (!shaders.load())
    {
        e_debug("load shaders error");
        return EXIT_FAILURE;
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

        const auto x_color_lerp = lerp(0_npx, w, colors::blue, colors::red);

        for (auto x = 0_npx; x < w; x += dx)
        {
            const auto xy_color_lerp = lerp(0_npx, h, to_color(x_color_lerp(x)), colors::green);

            for (auto y = 0_npx; y < h; y += dy)
            {
                shaders.color(to_colorf(xy_color_lerp(y)));
                shaders.position(x, y);
                shaders.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




