#include "event_processors_container.h"

namespace os_windows
{
    namespace
    {
        constexpr bool processor_is_root(event_processor_view processor) noexcept
        {
            return processor.window.id == processor.id;
        }
    }

    event_processors_container& event_processors_container_global() noexcept
    {
        static event_processors_container map;
        return map;
    }

    bool event_processors_container::erase(event_processor_view processor) noexcept
    {
        if ( const auto items = find(processor); items.first != items.last )
        {
            ++revision_;

            if ( processor_is_root(processor) )
            {
                as_mutable(items.first->value).ignore_callback();
            }
            else
            {
                map_.erase(items.first);
            }

            return true;
        }

        return false;
    }

    size_t event_processors_container::erase(const_window_handle_t window_handle) noexcept
    {
        const auto old_revision = revision_++;

        const auto count = map_.erase(window_handle);
        if ( !count )
        {
            revision_ = old_revision;
        }

        return count;
    }

    void event_processors_container::reset() noexcept
    {
        if ( map_.size() || map_.capacity() > map_.small_size )
        {
            ++revision_;
            map_.clear();
            map_.shrink_to_fit();
        }
    }

    size_t event_processors_container::insert_root(const_window_handle_t key) noexcept
    {
        const auto items = lower_bound(key);

        const auto id = ++revision_;

        map_.unsafe_force_insert_hint
        (
            items.first,
            key_value_type
            {
                key,
                item_event_processor
                {
                    id,
                    nullptr,
                    event_callback_state::ignored
                }
            }
        );

        return id;
    }

    size_t event_processors_container::insert(window_view window, event_callback_factory_t callback_factory) noexcept
    {
        if ( const auto items = find(window); items.first != items.last )
        {
            const auto next_revision = ++revision_;
            const auto root_is_unused = items.first->value.has_ignored_state();
            const auto id = ( root_is_unused ) ? window.id : next_revision;

            item_event_processor value
            {
                id,
                callback_factory(event_processor_view{ window, id }),
                event_callback_state::ready
            };

            if ( value.has_callback() )
            {
                if ( root_is_unused )
                {
                    as_mutable(items.first->value) = std::move(value);
                }
                else
                {
                    map_.unsafe_force_insert_hint
                    (
                        left_by_key(items, window.handle).last,
                        key_value_type
                        {
                            window.handle,
                            std::move(value)
                        }
                    );
                }
            }

            return id;
        }

        return 0u;
    }
}
