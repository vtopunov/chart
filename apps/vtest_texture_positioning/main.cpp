#include <debug/debug.h>

#include <egl_ui/egl_ui_owner.h>

#include <utility/png.h>
#include <utility/shader_library.h>


int app_main(os::module_handle_t app) noexcept
{
    const auto egl = create_egl_ui(app);
    if (!egl)
    {
        e_debug("create window error: window error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    const auto texture = png_texture_from_asset(_PATH("grid_9x9.png"));
    if (!texture)
    {
        e_debug("create png texture error: {}", glGetError());
        return EXIT_FAILURE;
    }

    shader_library<vert::positioned_texture, frag::default_texture> shaders{};
    if (!shaders.build())
    {
        e_debug("build shaders program error");
        return EXIT_FAILURE;
    }

    shaders.use();
    shaders.vert.u_size.store(sizes(texture));
    shaders.vert.u_viewport.store(viewport(egl));
    shaders.frag.s_texture.store(texture);

    {
        const egl_painting_owner painting_owner{ egl };
        gl::viewport(viewport(egl));
        gl::clear(colors::white_f);

        const auto vb = shaders.vert.a_frame.bind();

        const auto [w, h] = viewport(egl);
        const auto dx = width(texture) + 1_npx;
        const auto dy = height(texture) + 1_npx;

        for (pxsize_t y = 0_npx; y < h; y += dy)
        {
            for (pxsize_t x = 0_npx; x < w; x += dx)
            {
                shaders.vert.u_position.store(x, y);
                vb.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




