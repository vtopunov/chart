#include <core/utility.h>

#include <egl/ui_wrapper.h>

#include <android/sensor.h>

#include <entry_point/android_native_app_glue.h>

namespace egl
{
    namespace
    {
        ANativeWindow* receive_window(os::module_handle_t app) noexcept
        {
            constexpr int max_timeout_ms = 3000;

            while (!(app->destroyRequested))
            {
                if (const auto window = app->window)
                {
                    return window;
                }

                int events{};
                android_poll_source* source{ nullptr };

                const auto ident = ALooper_pollOnce(max_timeout_ms, nullptr, &events, (void**)&source);
                if (ident >= 0)
                {
                    if (source && source->process)
                    {
                        source->process(app, source);
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

            return nullptr;
        }

        template<class T>
        constexpr pxside_t to_px(T value) noexcept
        {
            constexpr T zero{};
            return narrow_cast<pxside_t>(std::max(zero, value));
        }
    }

    os::window_handle_t render_window(os::module_handle_t app) noexcept
    {
        return app->window;
    }

    void ui_resources_collector::operator()(const ui_resources& ui) const noexcept
    {
        if (ui.sensor_event_queue)
        {
            ASensorManager_destroyEventQueue(ui.sensor_manager, ui.sensor_event_queue);
        }
    }

    ui_wrapper ui_intance(os::module_handle_t app) noexcept
    {
        ui_wrapper result;

        auto& ui = as_mutable(result.r());

        ui.app = app;
        ui.sensor_manager = ASensorManager_getInstance();

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
            const auto window = receive_window(app);
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