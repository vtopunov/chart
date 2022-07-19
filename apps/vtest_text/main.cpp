#include <core/lerp.h>

#include <debug/debug.h>

#include <egl_ui/run.h>

#include <file/file_mmap.h>

#include <font/font.h>

#include <utility/shaders_library.h>

#ifdef D_OS_WINDOWS
#define D_FONT_NAME(name) _PATH("..\\fonts\\" ## name)
#else
#define D_FONT_NAME(name) _PATH(name)
#endif


using namespace std::string_view_literals;

namespace
{
    gl::texture2d text_rendering() noexcept
    {
        pix8map image{ 300_px, 60_px };
        if (!image)
        {
            e_debug("out of memory");
            return {};
        }

        const auto font_file = file::mmap(D_FONT_NAME("DroidSerif-Regular.ttf"));
        if (!font_file)
        {
            e_debug("can't open font file");
            return {};
        }

        const auto face = font::create_face(font_file, 20_px);

        if (!face)
        {
            e_debug("can't create font");
            return {};
        }

        
        {
            using namespace std::string_literals;
            draw_text(image, 3_px, 20_px, face, u8"Привет мир !_!`"s);
        }

        size(face, 15_px);

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
                (image.height() + tm.bottom - tm.top) / 2u,
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

int app_main(os::module_handle_t app) noexcept
{
    const auto egl = egl_instance(app);
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    const auto texture = text_rendering();
    if (!texture)
    {
        e_debug("text rendering fail");
        return EXIT_FAILURE;
    }

    shaders_library<vert::positioned_texture, frag::gray_texture_mix_color> shaders;
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
        egl_painting_owner painting_lock{ egl };
        gl::clear(gl::colors::white_f);

        const auto vb = shaders.vert.a_frame.bind();

        const auto [w, h] = sizes(egl);
        const auto dx = width(texture) + 1_px;
        const auto dy = height(texture) + 1_px;

        const auto y_color_lerp = lerp(num_range{ 0_px, h }, num_range{ colors::red, colors::blue });

        for (pxside_t y = 0; y < h; y += dy)
        {
            const auto yx_color_lerp = lerp(num_range{ 0_px, w }, num_range{ colors::green, color_cast<rgba_color32_t>(y_color_lerp(y)) });

            for (pxside_t x = 0; x < w; x += dx)
            {
                shaders.frag.u_color.store(gl::to_colorf(yx_color_lerp(x)));
                shaders.vert.u_position.store(point2d{ x, y });

                vb.draw();
            }
        }
    }

    return run(egl);
}




