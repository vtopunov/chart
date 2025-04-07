#include <debug/debug.h>

#include <egl_ui/egl_ui_owner.h>

#include <utility/png.h>
#include <shader/library.h>


int app_main(os::module_handle_t app) noexcept
{
    const auto egl = create_egl_ui(app);
    if (!egl)
    {
        e_debug("create window error: {}", egl_ui::error_code());
        return EXIT_FAILURE;
    }

    const auto texture = png_texture_from_asset(_PATH("grid_9x9.png"));
    if (!texture)
    {
        e_debug("create png texture error: {}", glGetError());
        return EXIT_FAILURE;
    }

    shader_embed::default_texture shaders{};
    if (!shaders.load())
    {
        e_debug("error loading shaders");
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

        for (npx_t y = 0_npx; y < h; y += dy)
        {
            for (npx_t x = 0_npx; x < w; x += dx)
            {
                shaders.position(x, y);
                shaders.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




