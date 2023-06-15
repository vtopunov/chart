#pragma once

#include <core/small_vector.h>

#include <ui/event_processor_fwd.h>


namespace ui
{
    struct event_processor_note
    {
        static constexpr window_handle_t garbage_mark{ nullptr };

        [[nodiscard]]
        std::optional<event_result_t> operator () (const event& e) const noexcept
        {
            return callback(e);
        }

        constexpr void mark_as_garbage() noexcept
        {
            window = garbage_mark;
        }

        event_callback_t callback;
        window_handle_t window;
        event_processor_resource processor;
    };


    class event_processors_storage
    {
        using self = event_processors_storage;

    public:
        constexpr event_processors_storage() noexcept = default;
        D_DISABLE_COPY_MOVE(event_processors_storage);

        static constexpr size_t static_size{ 8_uz };
        using vector_type = small_vector<event_processor_note, static_size>;
        using back_vector_type = small_vector<event_processor_note, 1_uz>;

        bool destroy_processor(event_processor_resource processor) noexcept;

        size_t close_window(window_handle_t window) noexcept;

        void reset() noexcept;

        [[nodiscard]] event_processor_resource create(window_handle_t window, event_callback_t callback) noexcept;

        class locked_storage
        {
        public:
            using const_iterator = vector_type::const_iterator;

            constexpr explicit locked_storage(self& container) noexcept
                : container_{ container }
            {}

            D_DISABLE_COPY_MOVE(locked_storage);

            ~locked_storage() noexcept
            {
                container_.unlock_and_collecting();
            }

            [[nodiscard]]
            constexpr const_iterator begin() const noexcept
            {
                return container_.items_.cbegin();
            }

            [[nodiscard]]
            constexpr const_iterator end() const noexcept
            {
                return container_.items_.cend();
            }

        private:
            self& container_;
        };

        [[nodiscard]]
        locked_storage lock() noexcept
        {
            lock_.lock();
            return locked_storage{ *this };
        }

    private:
        void unlock_and_collecting() noexcept;

        void update_first_garbage(event_processor_note* new_value) noexcept;


    private:
        vector_type items_{};
        back_vector_type back_items_{};

        event_processor_note* first_garbage_{ nullptr };


        class recursive_lock
        {
        public:
            constexpr recursive_lock() noexcept = default;
            D_DISABLE_COPY_MOVE(recursive_lock);

            constexpr void lock() noexcept
            {
                ++counter_;
            }

            constexpr bool unlock() noexcept
            {
                return !--counter_;
            }

            constexpr explicit operator bool() const noexcept
            {
                return !!counter_;
            }

        private:
            size_t counter_{ 0_uz };
        };

        recursive_lock lock_{};

        class desctiptor_generator
        {
        public:
            constexpr desctiptor_generator() noexcept = default;
            D_DISABLE_COPY_MOVE(desctiptor_generator);

            constexpr event_processor_resource operator () () noexcept
            {
                return underlying_cast<event_processor_resource>(++current_);
            }

        private:
            std::underlying_type_t<event_processor_resource> current_{ to_underlying(event_processor_resource::null) };
        };

        desctiptor_generator desctiptor_generator_{};
    };

    [[nodiscard]]
    event_processors_storage& event_processors_global() noexcept;
}