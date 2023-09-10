#pragma once

#include <gl/color.h>

#include <egl_ui/egl_ui_owner.h>

#include <widget/fwd.h>


namespace widget
{
    namespace colors
    {
        using namespace ::color_literals;

        constexpr auto dialog_color = 0xf0f0f0_rgb;
        constexpr auto gl_dialog_color_f = gl::to_colorf(dialog_color);
    }

    struct window : egl_ui_owner
    {};

    struct window_builder : egl_ui::egl_ui_gatherer<window_builder>
    {
#ifdef D_OS_WINDOWS
        window_builder() noexcept
        {
            background(ui::create_brush(colors::dialog_color));
            D_ASSERT(background());
        }
#endif

        [[nodiscard]]
        window build() const noexcept
        {
#ifdef D_OS_WINDOWS
            if (background()) [[likely]]
            {
                return { create_egl_ui(_c_params()) };
            }

            return {};
#else
            return { create_egl_ui(_c_params()) };
#endif
        }
    };
}