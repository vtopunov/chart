#pragma once

#include <core/null.h>

#include <os/fwd.h>

#if defined(D_OS_ANDROID)
#include <ui/app.h>
#endif

#include <egl_ui/viewport_rectangle.h>


namespace egl_ui
{
    struct window_resource
    {
        struct null_type
        {
            [[nodiscard]]
            constexpr operator window_resource() const noexcept
            {
                return window_resource
                {
                        .app{ nullptr },

    #if defined(D_OS_WINDOWS)
                        .app_wnd{ nullptr },
                        .render_wnd{ nullptr },

    #elif defined(D_OS_ANDROID)
                        .sensor_manager{ nullptr },
                        .sensor_event_queue{ nullptr },

    #endif
                        .window_viewport{ .sizes{ 0_px, 0_px } },
                };
            }
        };

        os::const_module_handle_t app;

#if defined(D_OS_WINDOWS)
        os::window_handle_t app_wnd;
        os::window_handle_t render_wnd;

#elif defined(D_OS_ANDROID)
        os::sensor_manager_handle_t sensor_manager;
        os::sensor_event_queue_handle_t sensor_event_queue;

#endif

        viewport_rectangle window_viewport;

        constexpr explicit operator bool() const noexcept
        {
            return !!window_viewport;
        }

        constexpr operator viewport_rectangle() const noexcept
        {
            static_assert(sizeof(viewport_rectangle) <= std::min(8_uz, 2u * sizeof(size_t)));
            return window_viewport;
        }

        constexpr operator os::const_module_handle_t() const noexcept
        {
            return app;
        }
    };

    using nullui_t = null_t<window_resource>;
    constexpr nullui_t nullui = null_v<window_resource>;


#if defined(D_OS_WINDOWS)
    [[nodiscard]]
    constexpr os::window_handle_t app_window(const window_resource& ui) noexcept
    {
        return ui.app_wnd;
    }

    [[nodiscard]]
    constexpr os::window_handle_t render_window(const window_resource& ui) noexcept
    {
        return ui.render_wnd;
    }

#elif defined(D_OS_ANDROID)
    [[nodiscard]]
    inline os::window_handle_t app_window(const window_resource& w) noexcept
    {
        return  ui::app_window(w.app);
    }

    [[nodiscard]]
    inline os::window_handle_t render_window(const window_resource& w) noexcept
    {
        return  ui::app_window(w.app);
    }

#endif
}