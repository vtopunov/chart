#pragma once

#include <egl_ui/egl_resources_fwd.h>
#include <egl_ui/ui_resources.h>
#include <egl_ui/egl_descriptors.h>

namespace egl_ui
{
    struct egl_resources
    {
        struct null_type
        {
            [[nodiscard]]
            constexpr operator egl_resources() const noexcept
            {
                return egl_resources
                {
                    .ui = nullui,
                    .display{ nullptr },
                    .surface{ nullptr },
                    .context{ nullptr }
                };
            }
        };

        ui_resources ui;

        display_descriptor_t display;
        surface_descriptor_t surface;
        context_descriptor_t context;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!context;
        }
    };

    [[nodiscard]]
    constexpr px::size2d sizes(const egl_resources& egl) noexcept
    {
        return egl.ui.sizes;
    }

    [[nodiscard]]
    constexpr pxside_t width(const egl_resources& egl) noexcept
    {
        return egl.ui.sizes.width();
    }

    [[nodiscard]]
    constexpr pxside_t height(const egl_resources& egl) noexcept
    {
        return egl.ui.sizes.height();
    }

    [[nodiscard]]
    constexpr os::module_handle_t app(const egl_resources& egl) noexcept
    {
        return app(egl.ui);
    }

    [[nodiscard]]
    D_CONDITIONAL_OS_WINDOWS(constexpr, inline) os::window_handle_t render_window(const egl_resources& egl) noexcept
    {
        return render_window(egl.ui);
    }
}
