#include "ui_wrapper.h"

#include <ui/window.h>

namespace egl
{
    namespace
    {
        constexpr bool pxsize_is_valid(pxside_t size) noexcept
        {
            return size > 0_px;
        }

        constexpr bool pxsize_is_valid(px::size2d sizes) noexcept
        {
            return pxsize_is_valid(sizes.width()) 
                && pxsize_is_valid(sizes.height());
        }
    }

    void close(const ui_resources& ui) noexcept
    {
        ui::close(ui.render_wnd);
        ui::close(ui.app_wnd);
    }

    ui_wrapper ui_intance(os::module_handle_t module) noexcept
    {
        ui_wrapper result;

        auto& w = as_mutable(result.r());

        ui::window_builder builder;
        builder.module(module);

        w.app_wnd = builder.build().release();

        if (w.app_wnd)
        {
            w.sizes = ui::desktop_sizes();
        }

        if (pxsize_is_valid(w.sizes))
        {
            w.render_wnd
                = builder
                .parent(w.app_wnd)
                .sizes(w.sizes)
                .build()
                .release();
        }

        return result;
    }
}