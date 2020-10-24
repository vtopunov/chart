#pragma once

#include <core/small_vector.h>
#include <core/utility.h>

#include <display/event_processor.h>

namespace display
{
    namespace event_processor
    {
        inline constexpr window_resource garbage_mark{ nullwindow };

        struct processor_note
        {
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
            processor_resource processor;
            void* data;
            event_callback_t callback;
        };

        class processors_container
        {
            using self = processors_container;

        public:
            using container_type = small_vector<processor_note, 8>;

            bool erase(processor_resource processor) noexcept;

            size_t erase(window_resource window) noexcept;

            void reset() noexcept;

            [[nodiscard]]
            processor_resource insert(window_resource window, void* data, event_callback_t callback) noexcept;

            class locked_container
            {
            public:
                constexpr locked_container(self* container) noexcept
                    : container_{ container }
                {}

                D_DISABLE_COPY_MOVE(locked_container)

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
                if ( !--lock_ )
                {
                    unlock_erase(garbage_mark);
                }
            }

        private:
            container_type items_;
            size_t lock_{ 0u };
        };

        [[nodiscard]]
        processors_container& processors_container_global() noexcept;
    }
}