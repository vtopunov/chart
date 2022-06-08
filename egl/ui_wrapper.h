#pragma once

#include <core/resouce.h>

#include <px/pxfwd.h>
#include <os/osfwd.h>

namespace egl
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
#if defined(D_OS_WINDOWS)
                    .app_wnd{ nullptr },
                    .render_wnd{ nullptr },

#elif defined(D_OS_ANDROID)
                    .app{ nullptr },
                    .sensor_manager{ nullptr },
                    .sensor_event_queue{ nullptr },

#endif
                    .sizes{ 0_px, 0_px },
                };
            }
        };

#if defined(D_OS_WINDOWS)
        os::window_handle_t app_wnd;
        os::window_handle_t render_wnd;
        
#elif defined(D_OS_ANDROID)
        os::module_handle_t app;
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
#if defined(D_OS_WINDOWS)
            return !!render_wnd;

#elif defined(D_OS_ANDROID)
            static_assert(std::is_unsigned_v<decltype(sizes.height())>);
            return !!sizes.height();

#endif
        }
    };
   
    using nullui_t = null_t<ui_resources>;
    constexpr nullui_t nullui = null_v<ui_resources>;

#if defined(D_OS_WINDOWS)
    [[nodiscard]]
    constexpr os::window_handle_t render_window(const ui_resources& ui) noexcept
    {
        return ui.render_wnd;
    }

#elif defined(D_OS_ANDROID)
    [[nodiscard]]
    os::window_handle_t render_window(const ui_resources& ui) noexcept;

    void quit(os::module_handle_t app) noexcept;

#endif

    struct ui_resources_collector
    {
        void operator () (const ui_resources& ui) const noexcept;
    };

    using ui_wrapper = unique_resource<ui_resources, ui_resources_collector>;

    [[nodiscard]]
    ui_wrapper ui_intance(os::module_handle_t module) noexcept;
}