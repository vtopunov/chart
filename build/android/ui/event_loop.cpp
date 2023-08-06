#include <ui/event_loop.h>

#include <entry_point/android_native_app_glue.h>

namespace ui
{
    namespace private_detail_event_loop
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

        message::process_result message::process(module_handle_t app) const noexcept
        {
            if (source_) [[likely]]
            {
                D_ASSERT(source_->process);
                source_->process(app, source_);
            }

            if (app->destroyRequested || window_ != app->window) [[unlikely]]
            {
                return process_result::quit;
            }

            return process_result::continue_processing;
        }
        
        app_manager::~app_manager() noexcept
        {
            quit(app_);
            receive_quit(app_);
            set_cmd_callback(app_, nullptr);
            set_input_event_callback(app_, nullptr);
            set_user_data(app_, nullptr);
        }
    }
}