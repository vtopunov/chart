#include <ui/event_loop.h>

#include <entry_point/android_native_app_glue.h>

namespace ui
{
    namespace private_detail_event_loop
    {
        bool message::process(module_handle_t app) const noexcept
        {
            static_assert(cmd_event_style::redraw_needed == to_cmd_event_style(APP_CMD_WINDOW_REDRAW_NEEDED));
            static_assert(cmd_event_style::content_rect_changed == to_cmd_event_style(APP_CMD_CONTENT_RECT_CHANGED));

            if (source_) [[likely]]
            {
                D_ASSERT(source_->process);
                source_->process(app, source_);
            }

            return !app->destroyRequested && (window_ == app->window);
        }
    }
}