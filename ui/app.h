#pragma once

#include <core/resource.h>

#include <debug/debug.h>

#include <ui/fwd.h>


namespace ui
{
#ifdef D_OS_ANDROID
    [[nodiscard]]
    window_handle_t app_window_handle(const_module_handle_t app) noexcept;

    using cmd_callback_t = void (*)(os::module_handle_t, int32_t);
    using input_event_callback_t = int32_t(*)(os::module_handle_t, AInputEvent*);

    [[nodiscard]]
    void* user_data(const_module_handle_t app) noexcept;
    void set_user_data(module_handle_t app, void* data) noexcept;
    void set_cmd_callback(module_handle_t app, cmd_callback_t callback) noexcept;
    void set_input_event_callback(module_handle_t app, input_event_callback_t callback) noexcept;

    void quit(const_module_handle_t app) noexcept;

    struct sensor_event_queue_resource
    {
        struct null_type
        {
            constexpr operator sensor_event_queue_resource () const noexcept
            {
                return
                {
                    .sensor_manager{ nullptr },
                    .sensor_event_queue{ nullptr }
                };
            }
        };

        constexpr explicit operator bool() const noexcept
        {
            return !!sensor_event_queue;
        }

        sensor_manager_handle_t sensor_manager;
        sensor_event_queue_handle_t sensor_event_queue;
    };

    static_assert(std::is_same_v<decl_null_type_t<sensor_event_queue_resource>, sensor_event_queue_resource::null_type>);

    struct sensor_event_queue_resource_collector
    {
        void operator () (sensor_event_queue_resource r) const noexcept;
    };

    using unique_sensor_event_queue
        = unique_resource<sensor_event_queue_resource, sensor_event_queue_resource_collector>;

    [[nodiscard]]
    unique_sensor_event_queue create_sensor_event_queue(const_module_handle_t app) noexcept;

#else
    void quit() noexcept;

    inline void quit(const_module_handle_t) noexcept
    {
        quit();
    }

#endif

    using error_code_t = D_OS_WINDOWS_OR(dword_t, int);

    [[nodiscard]]
    error_code_t error_code() noexcept;

    template<class FormatString, class... Args>
    void ui_fatal_debug(const_module_handle_t app, const FormatString& format_string, const Args&... args) noexcept
    {
        fatal_debug(format_string, args...);
        quit(app);
    }
}

using ui::ui_fatal_debug;
