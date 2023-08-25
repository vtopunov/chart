#pragma once

#include <chrono>

#include <core/buffer_view.h>

#include <egl_ui/egl_ui_owner.h>

#include <widget/fwd.h>
#include <widget/shaders.h>


namespace widget
{
    namespace colors
    {
        using namespace ::color_literals;

        constexpr auto dialog_color = 0xf0f0f0_rgb;
        constexpr auto gl_dialog_color_f = gl::to_colorf(dialog_color);
    }

    struct window : egl_ui_owner
    {
        shaders shaders{};
        buffer_t temp_buffer{};

#ifdef D_OS_WINDOWS
        pxsize2d content_sizes_cache{};
        std::chrono::steady_clock::time_point redraw_time_cache{};
#endif

        constexpr operator buffer_view () const noexcept
        {
            return as_mutable(temp_buffer);
        }
    };

    [[nodiscard]]
    constexpr pxsize2d content_sizes(const window& w) noexcept
    {
        return D_CONDITIONAL_OS_WINDOWS(w.content_sizes_cache, w.viewport);
    }


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