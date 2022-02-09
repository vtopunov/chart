#include <core/color.h>

#include <os/debug.h>

#include <ui/event_loop.h>

#include <egl/window.h>

#include <utility/shaders_library.h>
#include <utility/font_cache.h>

namespace
{
    namespace widget_colors
    {
        constexpr auto dialog = 0xf0f0f0_glrgb;
        constexpr auto button_frame = 0xadadad_glrgb;
        constexpr auto button = 0xe1e1e1_glrgb;
    }

    namespace widget_sizes
    {
        constexpr size2d button_frame{ 1_px, 1_px };
    }

    struct rect_shading
    {
        inline static shaders_library<vert::positioned_rect, frag::default_color> shaders{};

        static bool initialize(const egl::window_resources& egl) noexcept
        {
            if (!shaders.build())
            {
                e_debug("build shaders program error");
                return false;
            }

            shaders.use();
            shaders.vert.u_viewport.store(egl::sizes(egl));

            return true;
        }

        px::point2d position{};
        px::size2d sizes{};

        void draw(gl::rgba_colorf_t colorf) const noexcept
        {
            shaders.use();
            shaders.vert.u_position.store(position);
            shaders.vert.u_size.store(sizes);
            shaders.frag.u_color.store(colorf);
            shaders.vert.a_frame.draw();
        }
    };

    struct button
    {
        egl::window egl{};
        rect_shading rect{};
        std::u8string text{};
        font_cache::face font{};
        gl::texture2d font_texture{};
        shaders_library<vert::positioned_texture, frag::gray_texture_mix_color> text_shaders{};

        constexpr rect_shading rect_without_frame() const noexcept
        {
            constexpr auto frame_sizes = widget_sizes::button_frame;
            return
            {
                .position{ rect.position + frame_sizes },
                .sizes{ rect.sizes - 2_px * frame_sizes }
            };
        }

        int run() noexcept
        {
            if (!egl)
            {
                egl = egl::window_factory{}.create();
            }

            if (!egl)
            {
                e_debug("create window error: window error: {}, egl error: {}",
                    ui::error_code(), eglGetError());
                return EXIT_FAILURE;
            }

            if (!is_maximum_resolution(egl))
            {
                e_debug("Instance of window is not high dpi. Add <dpiAware>true</dpiAware> in manifest.");
                return EXIT_FAILURE;
            }

            if (!rect.initialize(egl))
            {
                e_debug("create button error");
                return EXIT_FAILURE;
            }

            font = font_cache::load_font(_PATH("..\\fonts\\DroidSerif-Regular.ttf"), 20_px);
            if (!font)
            {
                e_debug("can't create font");
                return EXIT_FAILURE;
            }
            
            if (const auto lock = egl::begin_painting(egl))
            {
                namespace colors = widget_colors;

                gl::clear(colors::dialog);
                rect.draw(colors::button_frame);

                const auto client_rect = rect_without_frame();
                client_rect.draw(colors::button);

                if (!font_texture || client_rect.sizes != sizes(font_texture))
                {
                    font_texture.reset();

                    pix8map pixmap{ client_rect.sizes };

                    const point2d center{ client_rect.position / 2 };
                    font::draw_text(pixmap, center, font, text);
                }
            }

            ui::show(egl, ui::show_command::show_maximazed);
            
            return ui::run_event_loop(egl, *this);
        }
    };
}

int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    return button
    {
        .rect
        {
            .position{30_px, 50_px},
            .sizes{300_px, 200_px}
        },
        .text{ u8"button №1" }

    }
    .run();
}




