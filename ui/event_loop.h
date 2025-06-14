#pragma once

#include <chrono>

#include <ui/event_processor.h>
#include <ui/event_matching.h>


namespace ui
{
    using milliseconds = std::chrono::milliseconds;

#ifdef D_OS_ANDROID
    constexpr milliseconds infinite{ -1 };

#else
    static_assert(sizeof(milliseconds::rep) > 4);
    constexpr milliseconds infinite{ 0xffffffffLL };

#endif

    constexpr idle_event idle_event_v{};

    template<class T>
    [[nodiscard]] auto do_idle(T& processor) noexcept
        -> decltype(processor(idle_event_v))
    {
        return processor(idle_event_v);
    }

    [[nodiscard]]
    constexpr milliseconds do_idle(no_overload) noexcept
    {
        return infinite;
    }


#if defined(D_OS_WINDOWS)
    namespace private_detail_event_loop
    {
        constexpr auto msg_storage_size = 48_uz;
        constexpr uint_t pm_remove{ 1u };

        void message_wait_for(milliseconds timeout) noexcept;

        void process_message(const os::message_t* msg) noexcept;

        [[nodiscard]]
        event_style e_style(const os::message_t* msg) noexcept;

        [[nodiscard]]
        word_parameter_t word_parameter(const os::message_t* msg) noexcept;

        [[nodiscard]]
        inline int exit_status(const os::message_t* msg) noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
            return static_cast<int>(word_parameter(msg));
            D_WARNING_POP;
        }

        void sizes_initialization() noexcept;

        template<class T>
        int run_event_loop_impl(T& idle_processor) noexcept
        {
            std::byte msg_storage[msg_storage_size]{};
            const auto pmsg = reinterpret_cast<os::message_t*>(std::data(msg_storage));

            do
            {
                const milliseconds timeout{ do_idle(idle_processor) };

                if (timeout <= milliseconds::zero()) [[likely]]
                {
                    if (PeekMessageW(pmsg, nullptr, 0u, 0u, pm_remove))
                    {
                        process_message(pmsg);
                    }
                }
                else
                {
                    message_wait_for(timeout);

                    while (PeekMessageW(pmsg, nullptr, 0u, 0u, pm_remove))
                    {
                        process_message(pmsg);

                        if (event_style::quit == e_style(pmsg))
                        {
                            break;
                        }
                    }
                }

            }
            while (event_style::quit != e_style(pmsg));

            return exit_status(pmsg);
        }
    }


    struct default_event_binder
    {
        template<class EventTarget>
        [[nodiscard]] event_processor bind(EventTarget& target) const noexcept
        {
            return create_event_processor
            (
                window,
                event_match{ oref(target) }
            );
        }

        const window_handle_t window;
    };

    template<class EventSource, class EventTarget>
    int run_event_loop(const EventSource& source, EventTarget&& target) noexcept
    {
        using event_source_binder_type = event_binder_type_t<std::remove_cvref_t<EventSource>>;
        auto& target_ref = as_reference(target);
        const auto event_bind_holder = event_source_binder_type{ source }.bind(target_ref);
        private_detail_event_loop::sizes_initialization();
        return private_detail_event_loop::run_event_loop_impl(target_ref);
    }

#elif defined(D_OS_ANDROID)
    namespace private_detail_event_loop
    {
        struct _private_app;

        template<class Processor>
        struct processor_callbacks
        {
            static_assert(!std::is_reference_v<Processor>);

            static void cmd_callback(void* processor, const ui::cmd_event& cmd_e) noexcept
            {
                do_cmd_event_match(processor_ref(processor), cmd_e);
            }

            static void input_event_callback(void* processor, const ui::event& input_e) noexcept
            {
                do_event_match(processor_ref(processor), input_e);
            }

            [[nodiscard]]
            static constexpr Processor& processor_ref(void* processor) noexcept
            {
                return *static_cast<Processor*>(processor);
            }
        };

        using cmd_callback_t = std::decay_t<decltype(processor_callbacks<nothing>::cmd_callback)>;
        using input_event_callback_t = std::decay_t<decltype(processor_callbacks<nothing>::input_event_callback)>;

        struct user_data
        {
            void* const processor;
            const cmd_callback_t cmd_callback;
            const input_event_callback_t input_event_callback;
        };

        class message
        {
        public:
            explicit message(const user_data& user_data) noexcept;

            D_DISABLE_COPYMOVE_CA(message);

            [[nodiscard]]
            D_FORCEINLINE bool poll(ui::milliseconds timeout) noexcept
            {
                return ALooper_pollAll
                (
                    narrow<int>(timeout.count()),
                    nullptr,
                    &events_,
                    (void**)&source_
                ) >= 0;
            }

            [[nodiscard]]
            bool process() const noexcept;

        private:
            window_handle_t window_{ nullptr };
            _private_app* app_{ nullptr };
            android_poll_source* source_{ nullptr };
            int events_{ 0 };
        };

        template<class Processor>
        int run_event_loop(Processor& processor) noexcept
        {
            {
                using processor_callbacks_type = processor_callbacks<std::remove_reference_t<Processor>>;

                const user_data user_data_instance
                {
                    as_mutable_pointer(std::addressof(processor)),
                    processor_callbacks_type::cmd_callback,
                    processor_callbacks_type::input_event_callback
                };

                for (message msg{ user_data_instance }; !msg.poll(do_idle(processor)) || msg.process(); ) [[likely]]
                {}
            }

            return EXIT_SUCCESS;
        }
    }

    template<class T>
    int run_event_loop(const no_overload, T&& target) noexcept
    {
        return private_detail_event_loop::run_event_loop(as_reference(target));
    }

#endif

    template<class EventSource>
    int run_event_loop(const EventSource& source) noexcept
    {
        constexpr struct {} nop;
        return run_event_loop(source, nop);
    }
}