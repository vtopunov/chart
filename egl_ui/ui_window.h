#pragma once

#include <core/resource.h>

#include <egl_ui/ui_window_parametrs.h>
#include <egl_ui/ui_window_resource.h>


namespace egl_ui
{
    struct ui_window_resource_collector
    {
        void operator () (const ui_window_resource& ui) const noexcept;
    };

    struct ui_window : unique_resource<ui_window_resource, ui_window_resource_collector>
    {
        constexpr operator viewport_rectangle() const noexcept
        {
            return static_cast<viewport_rectangle>(r());
        }

        constexpr operator os::const_module_handle_t() const noexcept
        {
            return static_cast<os::const_module_handle_t>(r());
        }
    };


    [[nodiscard]]
    ui_window create_window(const ui_window_parametrs& param) noexcept;
}