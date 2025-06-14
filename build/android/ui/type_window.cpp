#include <ui/type_window.h>
#include <ui/debug.h>

#include <android/sensor.h>

#include <entry_point/android_native_app_glue.h>
#include <common/app.h>


namespace ui
{
    namespace
    {
        void _quit(const android_app* const app) noexcept
        {
            if (!app->destroyRequested)
            {
                if (app->activity)
                {
                    ANativeActivity_finish(app->activity);
                }
            }
        }

        void receive_quit(android_app* const app) noexcept
        {
            if (app)
            {
                constexpr size_t max_number_of_checks{ 255 };
                constexpr int retry_check_timeout_ms{ 500 };

                app->onAppCmd = nullptr;
                app->onInputEvent = nullptr;
                app->userData = nullptr;

                _quit(app);

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
    }

    void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
    {
        if (type)
        {
            receive_quit(common::app_own::release());
            ASensorManager_destroyEventQueue(type.sensor_manager, type.sensor_event_queue);
        }
    }

    type_window_builder::type_window_builder() noexcept = default;

    type_window_builder::type_window_builder(const type_window_builder&) noexcept = default;

    type_window_builder::~type_window_builder() noexcept = default;

    type_window_builder& type_window_builder::operator=(const type_window_builder&) noexcept = default;

    type_window_builder& type_window_builder::style(uint_t) noexcept
    {
        return *this;
    }

    type_window_builder& type_window_builder::background(stock_brush) noexcept
    {
        return *this;
    }

    type_window_builder& type_window_builder::background(unique_brush) noexcept
    {
        return *this;
    }

    type_window_builder& type_window_builder::window_procedure(wndproc_t proc) noexcept
    {
        return *this;
    }

    uint_t type_window_builder::style() const noexcept
    {
        return {};
    }

    const_brush_handle_t type_window_builder::background() const noexcept
    {
        return {};
    }

    unique_type_window type_window_builder::build() noexcept
    {
        unique_type_window result{};
        auto& r = as_mutable(result.r());

        r.sensor_manager = ASensorManager_getInstance();
        if (r.sensor_manager) [[likely]]
        {
            if (const auto app = common::app(); app && app->looper) [[likely]]
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
        }

        return result;
    }
    
    [[nodiscard]]
    error_code_t error_code() noexcept
    {
        return errno;
    }

    void quit() noexcept
    {
        if (const auto app = common::app())
        {
            _quit(app);
        }
    }
}
