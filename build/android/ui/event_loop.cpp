#include <ui/event_loop.h>

#include <entry_point/android_native_app_glue.h>

namespace ui
{
    namespace private_detail_event_loop
    {
        std::optional<int> message::process(module_handle_t app) const noexcept
        {
            if (source_)
            {
                D_ASSERT(source_->process);
                source_->process(app, source_);
            }

            if (app->destroyRequested || window_ != app->window)
            {
                return EXIT_SUCCESS;
            }

            return std::nullopt;
        }
    }
}