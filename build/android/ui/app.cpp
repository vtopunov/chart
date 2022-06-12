#include <ui/app.h>

#include <core/assert.h>

#include <entry_point/android_native_app_glue.h>

namespace ui
{
    namespace app
    {
        namespace
        {
            void wait_for_finish(os::module_handle_t app) noexcept
            {
                constexpr size_t max_number_of_checks{ 255 };
                constexpr int retry_check_timeout_ms{ 500 };

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

        os::window_handle_t window(os::module_handle_t app) noexcept
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
            app->onAppCmd = nullptr;
            app->onInputEvent = nullptr;

            if (!app->destroyRequested)
            {
                if (app->activity)
                {
                    ANativeActivity_finish(app->activity);
                }

                wait_for_finish(app);
            }
        }
    }

    error_code_t error_code() noexcept
    {
        return errno;
    }
}