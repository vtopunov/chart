#include <debug/debug.h>

#include <egl_ui/egl_ui_owner.h>

#include <utility/png.h>
#include <utility/shaders_library.h>


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

    shaders_library<vert::positioned_texture, frag::default_texture> shaders{};
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

        const auto dx = width(texture) + 1_px;
        const auto dy = height(texture) + 1_px;

        for (pxside_t y = 0_px; y < egl.viewport.height(); y += dy)
        {
            for (pxside_t x = 0_px; x < egl.viewport.width(); x += dx)
            {
                shaders.vert.u_position.store(x, y);
                vb.draw();
            }
        }
    }

    return ui::run_event_loop(egl);
}




