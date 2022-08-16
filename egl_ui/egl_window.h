#pragma once

#include <core/resouce.h>

#include <egl_ui/egl_window_resource.h>

namespace egl_ui
{
    struct egl_window_resource_collector
    {
        void operator () (const egl_window_resource& egl) const noexcept;
    };

    struct egl_window : unique_resource<egl_window_resource, egl_window_resource_collector>
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
    egl_window create_egl_window(os::module_handle_t module) noexcept;
}

using egl_ui::egl_window;
using egl_ui::create_egl_window;