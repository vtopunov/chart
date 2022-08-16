#pragma once

#include <core/resouce.h>

#include <egl_ui/window_resource.h>

namespace egl_ui
{
    struct window_resource_collector
    {
        void operator () (const window_resource& ui) const noexcept;
    };

    struct window : unique_resource<window_resource, window_resource_collector>
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
    window create_window(os::module_handle_t app) noexcept;
}