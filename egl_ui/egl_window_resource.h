#pragma once

#include <egl_ui/egl_window_resource_fwd.h>
#include <egl_ui/window_resource.h>
#include <egl_ui/egl_descriptors.h>

namespace egl_ui
{
    struct egl_window_resource
    {
        struct null_type
        {
            [[nodiscard]]
            constexpr operator egl_window_resource() const noexcept
            {
                return egl_window_resource
                {
                    .ui = nullui,
                    .display{ nullptr },
                    .surface{ nullptr },
                    .context{ nullptr }
                };
            }
        };

        window_resource ui;

        display_descriptor_t display;
        surface_descriptor_t surface;
        context_descriptor_t context;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }

        constexpr operator viewport_rectangle() const noexcept
        {
            return ui;
        }

        constexpr operator os::const_module_handle_t() const noexcept
        {
            return ui;
        }
    };


    [[nodiscard]]
    D_CONDITIONAL_OS_WINDOWS(constexpr, inline) os::window_handle_t render_window(const egl_window_resource& egl) noexcept
    {
        return render_window(egl.ui);
    }
}
