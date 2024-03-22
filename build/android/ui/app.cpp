#include <ui/app.h>

#include <android/sensor.h>

#include <entry_point/android_native_app_glue.h>


namespace ui
{
    window_handle_t app_window_handle(const_module_handle_t app) noexcept
    {
        return app->window;
    }

    void set_user_data(module_handle_t app, void* data) noexcept
    {
        app->userData = data;
    }

    void* user_data(const_module_handle_t app) noexcept
    {
        return app->userData;
    }

    void set_cmd_callback(module_handle_t app, cmd_callback_t callback) noexcept
    {
        app->onAppCmd = callback;
    }

    void set_input_event_callback(module_handle_t app, input_event_callback_t callback) noexcept
    {
        app->onInputEvent = callback;
    }

    void quit(const_module_handle_t app) noexcept
    {
        if (!app->destroyRequested)
        {
            if (app->activity)
            {
                ANativeActivity_finish(app->activity);
            }
        }
    }

    void sensor_event_queue_resource_collector::operator()(sensor_event_queue_resource r) const noexcept
    {
        if (r)
        {
            ASensorManager_destroyEventQueue(r.sensor_manager, r.sensor_event_queue);
        }
    }

    unique_sensor_event_queue create_sensor_event_queue(const_module_handle_t app) noexcept
    {
        unique_sensor_event_queue result{};
        auto& r = as_mutable(result.r());

        r.sensor_manager = ASensorManager_getInstance();
        if (r.sensor_manager && app && app->looper) [[likely]]
        {
            r.sensor_event_queue = ASensorManager_createEventQueue
            (
                r.sensor_manager,
                app->looper,
                LOOPER_ID_USER,
                nullptr,
                nullptr
            );
        }

        return result;
    }

    error_code_t error_code() noexcept
    {
        return errno;
    }
}