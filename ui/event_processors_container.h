#pragma once

#include <core/small_vector.h>

#include <ui/event_processor.h>

namespace ui
{
    struct event_processor_note
    {
        static constexpr window_resource garbage_mark = nullwindow;

        [[nodiscard]]
        std::optional<event_result_t> operator () (const event& e) const noexcept
        {
            return callback(data, e);
        }

        constexpr void mark_as_garbage() noexcept
        {
            window = garbage_mark;
        }

        window_resource window;
        event_processor_resource processor;
        void* data;
        event_callback_t callback;
    };

    class event_processors_container
    {
        using self = event_processors_container;

    public:
        using container_type = small_vector<event_processor_note, 8>;

        bool erase(event_processor_resource processor) noexcept;

        size_t erase(window_resource window) noexcept;

        void reset() noexcept;

        [[nodiscard]]
        event_processor_resource insert(window_resource window, void* data, event_callback_t callback) noexcept;

        class locked_container
        {
        public:
            constexpr locked_container(self* container) noexcept
                : container_{ container }
            {}

            D_DISABLE_COPY_MOVE(locked_container);

            ~locked_container() noexcept
            {
                container_->unlock_and_collecting();
            }

            struct enumerate
            {
                size_t position;
                const container_type* container;

                [[nodiscard]]
                constexpr explicit operator bool() const noexcept
                {
                    return position < std::size(*container);
                }

                constexpr enumerate& operator++() noexcept
                {
                    ++position;
                    return *this;
                }

                [[nodiscard]]
                constexpr container_type::const_reference operator*() const noexcept
                {
                    return (*container)[position];
                }
            };

            [[nodiscard]]
            constexpr enumerate begin() const noexcept
            {
                return { 0u, std::addressof(as_immutable(container_)->items_) };
            }

            [[nodiscard]]
            constexpr null_t<enumerate> end() const noexcept
            {
                return {};
            }

        private:
            self* container_;
        };

        [[nodiscard]]
        locked_container lock() noexcept
        {
            ++lock_;
            return this;
        }

    private:
        size_t unlock_erase(window_resource window) noexcept;

        void unlock_and_collecting() noexcept
        {
            if (!--lock_)
            {
                unlock_erase(event_processor_note::garbage_mark);
            }
        }

    private:
        container_type items_;
        size_t lock_{ 0u };
    };

    [[nodiscard]]
    event_processors_container& event_processors_global() noexcept;
}