#include <ui/app.h>

#include <core/assert.h>
#include <core/utility.h>
#include <core/narrow.h>


#include <android/sensor.h>

#include <entry_point/android_native_app_glue.h>


namespace ui
{
    namespace
    {
        void receive_quit(os::module_handle_t app) noexcept
        {
            constexpr size_t max_number_of_checks{ 255 };
            constexpr int retry_check_timeout_ms{ 500 };

            app->onInputEvent = nullptr;
            app->onAppCmd = nullptr;

            for (size_t loop_limit{ max_number_of_checks }; loop_limit && !(app->destroyRequested); --loop_limit)
            {
                int events{};
                android_poll_source* source{ nullptr };
                if (const auto ident = ALooper_pollAll(retry_check_timeout_ms, nullptr, &events, (void**)&source); ident >= 0)
                {
                    if (source && source->process)
                    {
                        source->process(app, source);
                    }
                }
            }

            D_ASSERT(app->destroyRequested);
        }
    }


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

    unique_moudle module_startup_request(module_handle_t module) noexcept
    {
        constexpr int max_timeout_ms = 3000;

        while (!(module->destroyRequested))
        {
            if (module->window)
            {
                return
                {
                    resource_construct,
                    module
                };
            }

            int events{};
            android_poll_source* source{ nullptr };

            const auto ident = ALooper_pollOnce(max_timeout_ms, nullptr, &events, (void**)&source);
            if (ident >= 0)
            {
                if (source && source->process)
                {
                    source->process(module, source);
                }
            }
            else
            {
                if (ALOOPER_POLL_TIMEOUT == ident || ALOOPER_POLL_ERROR == ident)
                {
                    break;
                }
            }
        }

        return {};
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

    unique_sensor_event_queue create_sensor_event_queue(const_module_handle_t app) noexcept
    {
        unique_sensor_event_queue result;
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

    app_owner app_startup_request(module_handle_t module) noexcept
    {
        app_owner result{ .sensor_event_queue{ create_sensor_event_queue(module)}, .app_module{} };

        if (result.sensor_event_queue) [[likely]]
        {
            result.app_module = module_startup_request(module);
        }

        return result;
    }

    error_code_t error_code() noexcept
    {
        return errno;
    }

    void module_resource_collector::operator()(module_resource r) const noexcept
    {
        if (r)
        {
            quit(r);
            receive_quit(r.app);
            set_cmd_callback(r.app, nullptr);
            set_input_event_callback(r.app, nullptr);
            set_user_data(r.app, nullptr);
        }
    }

    void sensor_event_queue_resource_collector::operator()(sensor_event_queue_resource r) const noexcept
    {
        if (r)
        {
            ASensorManager_destroyEventQueue(r.sensor_manager, r.sensor_event_queue);
        }
    }
}