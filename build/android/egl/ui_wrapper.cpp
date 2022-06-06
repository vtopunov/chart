#include <core/utility.h>

#include <egl/ui_wrapper.h>

#include <android/sensor.h>
#include <android_native_app_glue.h>

namespace egl
{
    namespace
    {
        using app_cmd_callback_t = void (*)(os::module_handle_t app, int32_t cmd);

        class app_cmd_callback_instance
        {
        public:
            D_DISABLE_COPY_MOVE(app_cmd_callback_instance);

            constexpr app_cmd_callback_instance(os::module_handle_t app) noexcept
                : app_{ app }
            {
                if (app)
                {
                    old_user_data_ = std::exchange(app->userData, &last_cmd_);
                    old_app_cmd_ = std::exchange(app->onAppCmd, app_cmd_callback);
                }
            }

            ~app_cmd_callback_instance() noexcept
            {
                if (app_)
                {
                    app_->onAppCmd = old_app_cmd_;
                    app_->userData = old_user_data_;
                }
            }

            constexpr int32_t last_cmd() const noexcept
            {
                return last_cmd_;
            }

        private:
            static constexpr void app_cmd_callback(os::module_handle_t app, int32_t cmd) noexcept
            {
                *(static_cast<int32_t*>(app->userData)) = cmd;
            }

        private:
            os::module_handle_t app_{ nullptr };
            void* old_user_data_{ nullptr };
            app_cmd_callback_t old_app_cmd_{ nullptr };
            int32_t last_cmd_{ -1 };
        };

        ANativeWindow* get_window(os::module_handle_t app) noexcept
        {
            constexpr int max_timeout_ms = 3000;

            if (app->window)
            {
                return app->window;
            }

            const app_cmd_callback_instance app_cmd{ app };

            while(!(app->destroyRequested))
            {
                int events{};
                android_poll_source* source{ nullptr };

                const auto ident = ALooper_pollOnce(max_timeout_ms, nullptr, &events, (void**)&source);
                if (ident >= 0)
                {
                    if (source)
                    {
                        source->process(app, source);

                        switch (app_cmd.last_cmd())
                        {
                        case APP_CMD_INIT_WINDOW:
                            return app->window;

                        case APP_CMD_TERM_WINDOW:
                        case APP_CMD_PAUSE:
                        case APP_CMD_STOP:
                        case APP_CMD_DESTROY:
                            return nullptr;

                        default:
                            break;
                        }
                    }
                }
                else
                {
                    switch (ident)
                    {
                    case ALOOPER_POLL_TIMEOUT:
                    case ALOOPER_POLL_ERROR:
                        return nullptr;

                    default:
                        break;
                    }
                }
            }
            
            return nullptr;
        }

        template<class T>
        constexpr pxside_t to_px(T value) noexcept
        {
            constexpr T zero{};
            return narrow_cast<pxside_t>(std::max(zero, value));
        }  
    }

    os::window_handle_t render_window(const ui_resources& ui) noexcept
    {
        return (ui.app) ? ui.app->window : nullptr;
    }

    void close(const ui_resources& ui) noexcept
    {
        if (ui.sensor_event_queue)
        {
            ASensorManager_destroyEventQueue(ui.sensor_manager, ui.sensor_event_queue);
        }

        if (ui.app && ui.app->activity)
        {
            ANativeActivity_finish(ui.app->activity);
        }
    }

    ui_wrapper ui_intance(os::module_handle_t app) noexcept
    {
        ui_wrapper result;

        auto& ui = as_mutable(result.r());
        ui.app = app;

        if (app)
        {
            ui.sensor_manager = ASensorManager_getInstance();
        }

        if (ui.sensor_manager)
        {
            ui.sensor_event_queue = ASensorManager_createEventQueue
            (
                ui.sensor_manager,
                app->looper,
                LOOPER_ID_USER,
                nullptr,
                nullptr
            );
        }

        if (ui.sensor_event_queue)
        {
            const auto window = get_window(app);
            if (window)
            {
                ui.sizes.ref_width() = to_px(ANativeWindow_getWidth(window));
            }

            if (ui.sizes.width())
            {
                ui.sizes.ref_height() = to_px(ANativeWindow_getHeight(window));
            }
        }

        return result;
    }
}