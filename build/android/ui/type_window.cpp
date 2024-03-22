#include <ui/type_window.h>
#include <ui/app.h>

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

        void collect(module_handle_t app) noexcept
        {
            if (app)
            {
                quit(app);
                receive_quit(app);
                set_cmd_callback(app, nullptr);
                set_input_event_callback(app, nullptr);
                set_user_data(app, nullptr);
            }
        }

        void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
        {
            collect(type.module);
            
            {
                constexpr sensor_event_queue_resource_collector collect{};
                collect(type.handle);
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

        type_window_builder& type_window_builder::module(module_handle_t module) noexcept
        {
            module_ = module;
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

        module_handle_t type_window_builder::module() const noexcept
        {
            return module_;
        }

        const_brush_handle_t type_window_builder::background() const noexcept
        {
            return {};
        }

        unique_type_window type_window_builder::build(wzstring_view) noexcept
        {
            return build();
        }

        unique_type_window type_window_builder::build() noexcept
        {
            return
            {
                resource_construct,
                create_sensor_event_queue(module_).release(),
                module_
            };
        }
    }