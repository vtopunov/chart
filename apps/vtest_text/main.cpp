#include <core/lerp.h>

#include <debug/debug.h>

#include <px/pixmap.h>

#include <egli/egli_owner.h>

#include <file/file_asset.h>

#include <font/font.h>

#include <shader/library.h>


using namespace std::string_view_literals;

namespace
{
    gl::texture2d text_rendering() noexcept
    {
        lumpixmap image{ 300_npx, 60_npx };
        if (!image)
        {
            e_debug("out of memory");
            return {};
        }

        const auto font_file = file::asset::mmap(_PATH("OpenSans-Regular.ttf"));
        if (!font_file)
        {
            e_debug("can't open font file");
            return {};
        }

        const auto face = font::create_face(font_file, 20_npx);
        if (!face)
        {
            e_debug("can't create font");
            return {};
        }
   
        {
            using namespace std::string_literals;
            draw_text(image, 3_npx, 20_npx, face, u8"Привет мир !_!`"s);
        }

        size(face, 15_npx);

        {
            constexpr auto c_text = u8"Правый верх";
            const auto tm = text_metrics(face, c_text);
            draw_text(image, image.width() - tm.width, -tm.top, face, c_text);
        }

        {
            constexpr auto zsv_text = L">>> Центр <<<";

            const auto tm = text_metrics(face, zsv_text);

            draw_text
            (
                image,
                (image.width() - tm.width) / 2u,
                (image.height() + (tm.bottom - tm.top)) / 2u,
                face,
                zsv_text
            );
        }

        {
            constexpr auto text1 = "____"sv;
            constexpr auto text2 = L"````"sv;
            constexpr auto text3 = u8"Правый низ"sv;

            const auto tm
                = text_metrics
                (
                    text_metrics
                    (
                        text_metrics(face, text1),
                        face, text2
                    ),
                    face, text3
                );

            if (!tm)
            {
                e_debug("invalid text metrics");
                return {};
            }

            auto cursor = draw_text
            (
                image,
                image.width() - tm.width,
                image.height() - tm.bottom,
                face,
                text1
            );

            cursor = draw_text(image, cursor, face, text2);
            cursor = draw_text(image, cursor, face, text3);
        }

        auto texture = gl::create_texture2d(image);
        if (!texture)
        {
            e_debug("create texture error: {}", glGetError());
            return {};
        }

        return texture;
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

    const auto texture = text_rendering();
    if (!texture)
    {
        e_debug("text rendering fail");
        return EXIT_FAILURE;
    }

    shader_embed::luminance_texture shaders{};
    if (!shaders.load())
    {
        e_debug("build shaders program error");
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

        const auto y_color_lerp = lerp(0_npx, h, colors::red, colors::blue);

        for (auto y = 0_npx; y < h; y += dy)
        {
            const auto yx_color_lerp = lerp(0_npx, w, colors::green, to_color(y_color_lerp(y)));

            for (auto x = 0_npx; x < w; x += dx)
            {
                shaders.color(to_colorf(yx_color_lerp(x)));
                shaders.position(x, y);
                shaders.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




