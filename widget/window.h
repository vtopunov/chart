#pragma once

#include <egl_ui/egl_window.h>

#include <widget/window_fwd.h>
#include <widget/shaders.h>

namespace widget
{
    struct window
    {
        const egl_window egl;
        shaders shaders;
        mutable buffer_t temp_buffer;

        constexpr operator viewport_rectangle() const noexcept
        {
            return egl;
        }

        constexpr operator os::const_module_handle_t() const noexcept
        {
            return egl;
        }
    };
}