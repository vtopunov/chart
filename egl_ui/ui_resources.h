#pragma once

#include <core/null.h>

#include <px/fwd.h>
#include <os/fwd.h>

#if defined(D_OS_ANDROID)
#include <ui/app.h>
#endif

namespace egl_ui
{
    struct ui_resources
    {
        struct null_type
        {
            [[nodiscard]]
            constexpr operator ui_resources() const noexcept
            {
                return ui_resources
                {
                        .app{ nullptr },

    #if defined(D_OS_WINDOWS)
                        .app_wnd{ nullptr },
                        .render_wnd{ nullptr },

    #elif defined(D_OS_ANDROID)
                        .sensor_manager{ nullptr },
                        .sensor_event_queue{ nullptr },

    #endif
                        .sizes{ 0_px, 0_px },
                };
            }
        };


        os::module_handle_t app;

#if defined(D_OS_WINDOWS)
        os::window_handle_t app_wnd;
        os::window_handle_t render_wnd;

#elif defined(D_OS_ANDROID)
        os::sensor_manager_handle_t sensor_manager;
        os::sensor_event_queue_handle_t sensor_event_queue;

        using view_type = os::module_handle_t;

        constexpr operator view_type () const noexcept
        {
            return app;
        }
#endif

        px::size2d sizes;

        constexpr explicit operator bool() const noexcept
        {
            static_assert(std::is_unsigned_v<decltype(sizes.height())>);
            return !!sizes.height();
        }
    };

    using nullui_t = null_t<ui_resources>;
    constexpr nullui_t nullui = null_v<ui_resources>;

    [[nodiscard]]
    constexpr os::module_handle_t app(const ui_resources& ui) noexcept
    {
        return ui.app;
    }

#if defined(D_OS_WINDOWS)
    [[nodiscard]]
    constexpr os::window_handle_t render_window(const ui_resources& ui) noexcept
    {
        return ui.render_wnd;
    }

#elif defined(D_OS_ANDROID)
    [[nodiscard]]
    inline os::window_handle_t render_window(const ui_resources& ui) noexcept
    {
        return  ui::app_window(app(ui));
    }

#endif
}