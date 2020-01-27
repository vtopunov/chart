#pragma once

#include <core/small_flat_map.h>

#include <platform/windows/defs.h>

namespace os_windows
{
    size_t generate_event_handler_id() noexcept;

    enum class event_callback_state
    {
        ignored,
        in_process,
        ready
    };

    class event_handler_item
    {
    public:
        event_handler_item() noexcept = default;

        event_handler_item(size_t id, event_callback_function callback, event_callback_state callback_state) noexcept
            : callback_{ std::move(callback) }
            , id_{ id }
            , callback_state_{ callback_state }
        {}

        event_result do_process_event(const event& current_event) const noexcept
        {
            return callback_(current_event);
        }

        constexpr size_t id() const noexcept
        {
            return id_;
        }

        constexpr event_callback_state callback_state() const noexcept
        {
            return callback_state_;
        }

    private:
        event_callback_function callback_{ ignore_event_callback };
        size_t id_{ 0u };
        event_callback_state callback_state_{ event_callback_state::ignored };
    };

    struct event_handler_container
    {
        using container_type = small_flat_map<const_window_handle_t, event_handler_item, 4>;
        using key_value_type = container_type::value_type;
        using key_type = container_type::key_type;
        using const_key_value_iterator = container_type::const_iterator;
        using const_key_value_iterator_range = iterator_range<const_key_value_iterator>;

        static constexpr const_key_value_iterator_range null_range{ nullptr, nullptr };

        const_key_value_iterator_range lower_bound(const_window_handle_t window_handle) const noexcept
        {
            return map_.lower_bound(window_handle);
        }

        const_key_value_iterator_range find(window_view window) const noexcept
        {
            if (window.handle && window.id)
            {
                const auto current = map_.lower_bound(window.handle);
                if (starts_with_key(current, window.handle) && current.first->value.id() == window.id)
                {
                    return current;
                }
            }

            return null_range;
        }

        const_key_value_iterator_range find(event_handler_view handler) const noexcept
        {
            if (handler.id)
            {
                for (auto current = find(handler.window); current.first != current.last; ++current.first)
                {
                    if (current.first->value.id() == handler.id)
                    {
                        return current;
                    }
                }
            }

            return null_range;
        }

        size_t revision() const noexcept
        {
            return revision_;
        }

        size_t erase(window_view root) noexcept
        {
            if (const auto items = find(root); items.first != items.last)
            {
                ++revision_;
                return map_.erase(left_by_key(items, root.handle));
            }
            return 0u;
        }

        bool erase(event_handler_view handler) noexcept
        {
            if (handler.is_root())
            {
                if (const auto items = find(handler.window); items.first != items.last)
                {
                    const auto& cvalue_ref = items.first->value;
                    if (cvalue_ref.callback_state() == event_callback_state::in_process)
                    {
                        ++revision_;
                    }

                    const_cast<event_handler_item&>(cvalue_ref) = event_handler_item
                    { 
                        handler.id, 
                        ignore_event_callback, 
                        event_callback_state::ready
                    };

                    return true;
                }
            }
            else
            {
                if (const auto items = find(handler); items.first != items.last)
                {
                    ++revision_;
                    map_.erase(items.first);
                    return true;
                }
            }

            return false;
        }

        size_t erase(const_window_handle_t window_handle) noexcept
        {
            const auto old_revision = revision_++;

            const auto count = map_.erase(window_handle);
            if (!count)
            {
                revision_ = old_revision;
            }

            return count;
        }

        void clear() noexcept
        {
            if (!map_.empty())
            {
                ++revision_;
                map_.clear();
            }
        }

        void replace(const_window_handle_t key, event_handler_item handler_item)
        {
            auto items = map_.lower_bound(key);

            ++revision_;

            map_.erase(left_by_key(items, key));

            map_.unsafe_force_insert_hint
            (
                items.first,
                key_value_type{ key, std::move(handler_item) }
            );
        }

        void insert(const_key_value_iterator position, const_window_handle_t key, event_handler_item handler_item) noexcept
        {
            ++revision_;
            map_.unsafe_force_insert_hint
            (
                position,
                key_value_type{ key, std::move(handler_item) }
            );
        }

    private:
        size_t revision_{ 0u };
        container_type map_;
    };

    event_handler_container& event_handler_container_global() noexcept;

    inline const event_handler_container& const_event_handler_container_global() noexcept
    {
        return std::as_const(event_handler_container_global());
    }
}