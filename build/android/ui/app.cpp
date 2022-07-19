#include <ui/app.h>

#include <core/assert.h>

#include <entry_point/android_native_app_glue.h>

namespace ui
{
    os::window_handle_t app_window(os::module_handle_t app) noexcept
    {
        return app->window;
    }

    void set_user_data(os::module_handle_t app, void* data) noexcept
    {
        app->userData = data;
    }

    void* user_data(module_handle_t app) noexcept
    {
        return app->userData;
    }

    void set_cmd_callback(os::module_handle_t app, cmd_callback_t callback) noexcept
    {
        app->onAppCmd = callback;
    }

    void set_input_event_callback(os::module_handle_t app, input_event_callback_t callback) noexcept
    {
        app->onInputEvent = callback;
    }

    void quit(module_handle_t app) noexcept
    {
        if (!app->destroyRequested)
        {
            if (app->activity)
            {
                ANativeActivity_finish(app->activity);
            }
        }
    }

    error_code_t error_code() noexcept
    {
        return errno;
    }
}