#pragma once

#include <core/small_flat_map.h>

#include <platform/windows/event_processor.h>

namespace os_windows
{
    enum class event_callback_state
    {
        ignored,
        in_process,
        ready
    };

    class item_event_processor
    {
    public:
        static constexpr auto default_state = event_callback_state::ignored;

        item_event_processor() noexcept = default;

        item_event_processor(item_event_processor&& item) noexcept
            : callback{ std::move(item.callback) }
            , id_{ std::exchange(item.id_, 0u) }
            , state_{ std::exchange(item.state_, default_state) }
        {}

        item_event_processor& operator = (item_event_processor&& item) noexcept
        {
            callback.swap(item.callback);
            std::swap(id_, item.id_);
            std::swap(state_, item.state_);
            return *this;
        }

        item_event_processor(const item_event_processor&) noexcept = delete;

        item_event_processor& operator = (const item_event_processor&) noexcept = delete;

        item_event_processor(size_t id, event_callback_t callback, event_callback_state state) noexcept
            : callback{ std::move(callback) }
            , id_{ id }
            , state_{ state }
        {}

        constexpr size_t id() const noexcept
        {
            return id_;
        }

        constexpr event_callback_state state() const noexcept
        {
            return state_;
        }

        constexpr bool has_ready_state() const noexcept
        {
            return state_ == event_callback_state::ready;
        }

        constexpr bool has_ignored_state() const noexcept
        {
            return state_ == event_callback_state::ignored;
        }

        constexpr bool in_process() const noexcept
        {
            return state_ == event_callback_state::in_process;
        }

        bool has_callback() const noexcept
        {
            return callback != nullptr;
        }

        void ignore_callback() noexcept
        {
            state_ = event_callback_state::ignored;
            callback = nullptr;
        }

    public:
        class process_context
        {
        public:
            process_context(item_event_processor& item) noexcept
                : callback{ std::move(item.callback) }
            {
                item.state_ = event_callback_state::in_process;
            }

            std::optional<event_result_t> do_process(const event& current_event) const noexcept
            {
                return callback(current_event);
            }

            void move_to(item_event_processor& item) noexcept
            {
                item.state_ = event_callback_state::ready;
                item.callback = std::move(callback);
            }

        private:
            event_callback_t callback;
        };

    private:
        event_callback_t callback{ nullptr };
        size_t id_{ 0u };
        event_callback_state state_{ default_state };
    };

    struct event_processors_container
    {
        using container_type = small_flat_map<const_window_handle_t, item_event_processor, 8>;
        using key_value_type = container_type::key_value_type;
        using key_type = container_type::key_type;
        using const_key_value_iterator = container_type::const_iterator;
        using const_key_value_iterator_range = iterator_range<const_key_value_iterator>;

        static constexpr const_key_value_iterator_range null_range{ nullptr, nullptr };

        constexpr const_key_value_iterator_range lower_bound(const_window_handle_t window_handle) const noexcept
        {
            return map_.lower_bound(window_handle);
        }

        constexpr const_key_value_iterator_range find(window_view window) const noexcept
        {
            if ( window.handle && window.id )
            {
                const auto current = lower_bound(window.handle);
                if ( starts_with_key(current, window.handle) && current.first->value.id() == window.id )
                {
                    return current;
                }
            }

            return null_range;
        }

        constexpr const_key_value_iterator_range find(event_processor_view processor) const noexcept
        {
            if ( processor.id )
            {
                for ( auto current = find(processor.window); current.first != current.last; ++current.first )
                {
                    if ( current.first->value.id() == processor.id )
                    {
                        return current;
                    }
                }
            }

            return null_range;
        }

        constexpr size_t revision() const noexcept
        {
            return revision_;
        }

        bool erase(event_processor_view processor) noexcept;

        size_t erase(const_window_handle_t window_handle) noexcept;

        void reset() noexcept;

        size_t insert_root(const_window_handle_t key) noexcept;

        size_t insert(window_view window, event_callback_factory_t callback_factory) noexcept;

    private:
        size_t revision_{ 0u };
        container_type map_;
    };

    event_processors_container& event_processors_container_global() noexcept;

    inline const event_processors_container& const_event_processors_container_global() noexcept
    {
        return std::as_const(event_processors_container_global());
    }
}