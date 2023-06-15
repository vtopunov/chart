#include "ui_window.h"

#include <ui/window.h>

namespace egl_ui
{
    void ui_window_resource_collector::operator () (const ui_window_resource& ui) const noexcept
    {
        ui::close(ui.render_wnd);
        ui::close(ui.app_wnd);
    }

    ui_window create_window(const ui_window_parametrs& param) noexcept
    {
        ui_window result;
        auto& w = as_mutable(result.r());

        ui::window_builder builder{};
        builder
            .module(param.module)
            .geometry(param.geometry);

        const auto sizes = ui::desktop_sizes();
        static_assert(std::is_unsigned_v<decltype(sizes.width())>);
        static_assert(std::is_unsigned_v<decltype(sizes.height())>);
        if (sizes.width() && sizes.height())
        {
            w.app_wnd = builder.build().release();
        }

        if (w.app_wnd)
        {
            w.app = builder.module();
            D_ASSERT(w.app);
        }

        if (w.app)
        {
            w.render_wnd
                = builder
                .parent(w.app_wnd)
                .position(0_px, 0_px)
                .sizes(sizes)
                .build()
                .release();
        }

        if (w.render_wnd)
        {
            w.viewport_geometry.sizes = sizes;
        }

        return result;
    }
}