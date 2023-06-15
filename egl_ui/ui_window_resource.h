#pragma once

#include <core/null.h>

#include <os/fwd.h>

#if defined(D_OS_ANDROID)
#include <ui/app.h>
#endif

#include <egl_ui/viewport_rectangle.h>


namespace egl_ui
{
    struct ui_window_resource
    {
        struct null_type
        {
            [[nodiscard]]
            constexpr operator ui_window_resource() const noexcept
            {
                return ui_window_resource
                {
                        .app{ nullptr },

    #if defined(D_OS_WINDOWS)
                        .app_wnd{ nullptr },
                        .render_wnd{ nullptr },

    #elif defined(D_OS_ANDROID)
                        .sensor_manager{ nullptr },
                        .sensor_event_queue{ nullptr },

    #endif
                        .viewport_geometry{ .sizes{ 0_px, 0_px } },
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

        viewport_rectangle viewport_geometry;

        constexpr explicit operator bool() const noexcept
        {
            return !!viewport_geometry;
        }

        constexpr operator viewport_rectangle() const noexcept
        {
            static_assert(sizeof(viewport_rectangle) <= std::min(8_uz, 2u * sizeof(size_t)));
            return viewport_geometry;
        }

        constexpr operator os::const_module_handle_t() const noexcept
        {
            return app;
        }
    };

    using nullui_t = null_t<ui_window_resource>;
    static_assert(std::is_same_v<nullui_t, ui_window_resource::null_type>);
    constexpr nullui_t nullui{};


#if defined(D_OS_WINDOWS)
    [[nodiscard]]
    constexpr os::window_handle_t app_window(const ui_window_resource& ui) noexcept
    {
        return ui.app_wnd;
    }

    [[nodiscard]]
    constexpr os::window_handle_t render_window(const ui_window_resource& ui) noexcept
    {
        return ui.render_wnd;
    }

#elif defined(D_OS_ANDROID)
    [[nodiscard]]
    inline os::window_handle_t app_window(const ui_window_resource& w) noexcept
    {
        return  ui::app_window(w.app);
    }

    [[nodiscard]]
    inline os::window_handle_t render_window(const ui_window_resource& w) noexcept
    {
        return  ui::app_window(w.app);
    }

#endif
}