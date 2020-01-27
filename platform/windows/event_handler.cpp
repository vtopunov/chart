#include "event_handler.h"

#include <platform/windows/event_handler_container.h>

namespace os_windows
{
    namespace
    {
        using iterator_range = event_handler_container::const_key_value_iterator_range;
        using iterator = event_handler_container::const_key_value_iterator;

        struct position_for_write
        {
            iterator position;
            bool can_rewrite;
        };

        constexpr position_for_write find_position_for_write
        (
            iterator_range items,
            const_window_handle_t window_handle
        ) noexcept
        {
            for (; starts_with_key(items, window_handle); ++items.first)
            {
                if (items.first->value.callback_state() == event_callback_state::ignored)
                {
                    return { items.first, true };
                }
            }

            return { items.first, false };
        }

        size_t reuse_or_generate_id(const position_for_write& reuse_position) noexcept
        {
            return reuse_position.can_rewrite
                ? reuse_position.position->value.id()
                : generate_event_handler_id();
        }

        void write_to_event_handler_container
        (
            event_handler_container& map,
            position_for_write position,
            const_window_handle_t window_handle,
            event_handler_item item
        ) noexcept
        {
            if (position.can_rewrite)
            {
                const_cast<event_handler_item&>(position.position->value)
                    = std::move(item);
                return;
            }

            map.insert
            (
                position.position,
                window_handle,
                std::move(item)
            );
        }
    }

    bool event_handler::close() noexcept
    {
        const auto self = std::exchange(*this, {});
        return unregister_event_handler(self.view());
    }

    bool event_handler::is_valid() const noexcept
    {
        const auto position = const_event_handler_container_global().find(view_);
        return position.first != position.last;
    }

    safe_event_handler register_event_handler_factory(window_view window, event_callback_factory callback_factory) noexcept
    {
        auto& map = event_handler_container_global();
        if (const auto items = map.find(window); items.first != items.last)
        {
            const auto position_for_write
                = find_position_for_write(items, window.handle);

            const auto id = reuse_or_generate_id(position_for_write);

            auto result = make_unique_handle<event_handler>(window, id);

            auto callback = callback_factory(result->view());

            if (callback)
            {
                write_to_event_handler_container
                (
                    map,
                    position_for_write,
                    window.handle,
                    event_handler_item
                    {
                        id,
                        std::move(callback),
                        event_callback_state::ready
                    }
                );

                return result;
            }
        }

        return {};
    }

    safe_event_handler register_event_handler(window_view window, event_callback_function callback) noexcept
    {
        return register_event_handler_factory
        (
            window,
            [&callback] (event_handler_view) noexcept { return std::move(callback); }
        );
    }

    bool unregister_event_handler(event_handler_view view) noexcept
    {
        return event_handler_container_global().erase(view);
    }
}
