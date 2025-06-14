#include <ui/event_loop.h>

#include <entry_point/android_native_app_glue.h>
#include <common/app.h>


namespace ui
{
    static_assert(cmd_event_style::redraw_needed == to_cmd_event_style(APP_CMD_WINDOW_REDRAW_NEEDED));
    static_assert(cmd_event_style::content_rect_changed == to_cmd_event_style(APP_CMD_CONTENT_RECT_CHANGED));

    namespace private_detail_event_loop
    {
        struct _private_app : android_app
        {};

        namespace
        {
             constexpr const user_data& app_user_data(const android_app*const app) noexcept
             {
                 return *static_cast<const user_data*const>(app->userData);
             }
        }

        message::message(const user_data& user_data_ref) noexcept
            : app_{ static_cast<_private_app*>(common::app()) }
        {
            if (app_) [[likely]]
            {
                constexpr auto app_cmd_callback = [] (android_app* app, int32_t cmd) noexcept
                {
                    const ui::cmd_event cmd_e{ to_cmd_event_style(cmd) };
                    {
                        const auto& user_data_ref = app_user_data(app);
                        user_data_ref.cmd_callback(user_data_ref.processor, cmd_e);
                    }
                };

                constexpr auto app_input_callback = [] (android_app* app, AInputEvent* input_e) noexcept
                {
                    if (const ui::event e{ input_e })
                    {
                        const auto& user_data_ref = app_user_data(app);
                        user_data_ref.input_event_callback(user_data_ref.processor, input_e);
                    }

                    return 0;
                };

                window_ = app_->window;
                app_->userData = const_cast<user_data*>(std::addressof(user_data_ref));
                app_->onAppCmd = app_cmd_callback;
                app_->onInputEvent = app_input_callback;
            }
        }

        bool message::process() const noexcept
        {
            if (source_) [[likely]]
            {
                source_->process(app_, source_);
            }

            return !(app_->destroyRequested) && (window_ == app_->window);
        }
    }
}