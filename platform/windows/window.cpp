#include "window.h"

#include <core/unique_handle.h>
#include <platform/windows/event_handler_container.h>

namespace os_windows
{
    namespace
    {
        bool destroy_window(event_handler_container& handlers, const window_view window) noexcept
        {
            if (handlers.erase(window))
            {
                const auto ok
                    = DestroyWindow(window.handle) != FALSE;
                D_ASSERT(ok);
                return ok;
            }

            return false;
        }
    }

    bool window::is_valid() const noexcept
    {
        const auto items = const_event_handler_container_global().find(window_);
        return items.first != items.last;
    }

    bool window::show(int cmd) const noexcept
    {
        D_ASSERT(is_valid());
        return ShowWindow(window_.handle, cmd) != FALSE;
    }

    bool window::update() const noexcept
    {
        D_ASSERT(is_valid());
        return UpdateWindow(window_.handle) != FALSE;
    }

    bool window::close() noexcept
    {
        const auto self = std::exchange(*this, {});
        return destroy_window(event_handler_container_global(), self.view());
    }

    safe_window create_window(window_info info) noexcept
    {
        safe_window result;

        if (!info.type_)
        {
            info.type_ = register_window_type({});
        }

        if (info.type_)
        {
            const auto window_handle =
                CreateWindowExW
                (
                    0,
                    info.type_->name_id(),
                    info.title_.c_str(),
                    info.style_,
                    info.x_,
                    info.y_,
                    info.width_,
                    info.height_,
                    nullptr,
                    nullptr,
                    info.type_->module_address(),
                    nullptr
                );

            if (window_handle)
            {
                const auto root_event_hanlder_id = generate_event_handler_id();

                result = make_shared_handle<window>(std::move(info.type_), window_view{ window_handle, root_event_hanlder_id });

                event_handler_container_global().replace
                (
                    window_handle,
                    event_handler_item
                    { 
                        root_event_hanlder_id, 
                        ignore_event_callback, 
                        event_callback_state::ignored
                    }
                );
            }
        }

        return result;
    }
}