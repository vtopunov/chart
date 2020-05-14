#include "window.h"

#include <platform/windows/event_processors_container.h>

namespace os_windows
{
    namespace native_window_system
    {
        namespace
        {
            class children_container
            {
            public:
                using container_type = small_flat_map<const_window_handle_t, window_view, 6>;

                void reset() noexcept
                {
                    map_.clear();
                    map_.shrink_to_fit();
                }

                childrens_enumerator childrens(const_window_handle_t parent) const noexcept
                {
                    return
                    {
                        map_.lower_bound(parent),
                        parent
                    };
                }

                void close_childrens(const_window_handle_t parent) noexcept
                {
                    for ( ;;)
                    {
                        const auto childrens = map_.lower_bound(parent);

                        if ( !starts_with_key(childrens, parent) )
                        {
                            break;
                        }

                        const auto children = childrens.first->value;
                        map_.erase(childrens.first);
                        close(children);
                    }
                }

                void add(const_window_handle_t parent, window_view window) noexcept
                {
                    map_.force_insert(parent, window);
                }

            private:
                container_type map_;
            };

            children_container& children_container_global() noexcept
            {
                static children_container map;
                return map;
            }

            rect_t make_rect(const RECT& rect) noexcept
            {
                return make_rect
                (
                    make_point
                    (
                        narrow_cast<pixel_t>( rect.left ),
                        narrow_cast<pixel_t>( rect.top )
                    ),
                    make_point
                    (
                        narrow_cast<pixel_t>( rect.right ),
                        narrow_cast<pixel_t>( rect.bottom )
                    )
                );
            }

            bool close(window_handle_t handle) noexcept
            {
                close_childrens(handle);
                event_processors_container_global().erase(handle);
                return !!DestroyWindow(handle);
            }
        }

        void close_childrens(const_window_handle_t parent) noexcept
        {
            children_container_global().close_childrens(parent);
        }

        childrens_enumerator childrens(const_window_handle_t parent) noexcept
        {
            return children_container_global().childrens(parent);
        }

        rect_t client_rect(window_handle_t handle) noexcept
        {
            RECT rect{ 0, 0, 0, 0 };
            GetClientRect(handle, &rect);
            return make_rect(rect);
        }

        rect_t full_rect(window_handle_t handle) noexcept
        {
            RECT rect{ 0, 0, 0, 0 };
            GetWindowRect(handle, &rect);
            return make_rect(rect);
        }
    }

    void clear_global_state() noexcept
    {
        native_window_system::children_container_global().reset();
        event_processors_container_global().reset();
    }

    bool exist(window_view window) noexcept
    {
        const auto items = const_event_processors_container_global().find(window);
        return items.first != items.last;
    }

    bool show(window_view window, int cmd) noexcept
    {
        D_ASSERT(exist(window));
        return !!ShowWindow(window.handle, cmd);
    }

    bool update(window_view window) noexcept
    {
        D_ASSERT(exist(window));
        return !!UpdateWindow(window.handle);
    }

    bool close( window_view window) noexcept
    {
        if ( exist(window) )
        {
            const bool ok = native_window_system::close(window.handle);
            D_ASSERT(ok);
            return ok;
        }

        return false;
    }

    rect_t full_rect(window_view window) noexcept
    {
        D_ASSERT(exist(window));
        return native_window_system::full_rect(window.handle);
    }

    rect_t client_rect(window_view window) noexcept
    {
        D_ASSERT(exist(window));
        return native_window_system::client_rect(window.handle);
    }

    childrens_window_enumerator childrens(window_view window) noexcept
    {
        D_ASSERT(exist(window));
        return native_window_system::childrens(window.handle);
    }

    safe_window window_factory::create() const noexcept
    {
        const auto has_parent = ( parent_.handle != nullptr );
        D_ASSERT(!has_parent || exist(parent_));

        safe_window result;

        safe_window_type type{ type_ };

        const auto style = ( style_.has_value() )
            ? *style_
            : static_cast<DWORD>( has_parent ? WS_VISIBLE | WS_CHILD : WS_OVERLAPPEDWINDOW );

        if ( !type )
        {
            type = window_type_factory{}.create();

            if ( !type )
            {
                return result;
            }
        }

        const auto window_handle = CreateWindowExW
        (
            0,
            type->name_id,
            title_.c_str(),
            style,
            position_.x(),
            position_.y(),
            size_.width(),
            size_.height(),
            parent_.handle,
            nullptr,
            type->module_address,
            nullptr
        );

        if ( window_handle )
        {
            native_window_system::close_childrens(window_handle);
            event_processors_container_global().erase(window_handle);

            result = make_shared_handle<native_window_system::window_data>(
                window_view
                {
                    window_handle,
                    event_processors_container_global().insert_root(window_handle)
                },
                std::move(type)
            );

            if ( has_parent )
            {
                native_window_system::children_container_global().add(parent_.handle, result);
            }
        }

        return result;
    }
}