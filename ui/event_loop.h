#pragma once

#include <chrono>

#include <os/os_detection.h>

#include <ui/app.h>

#ifdef D_OS_WINDOWS
#include <ui/event_processor.h>
#endif

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
    constexpr milliseconds do_idle(no_overloaded) noexcept
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
                    ui::private_detail_event_loop::message_wait_for(timeout);

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
                event_match{ std::ref(target) }
            );
        }

        const window_handle_t window;
    };

    template<class T>
    using decl_event_binder_type_t = std::add_const_t<typename T::event_binder_type>;

    template<class T>
    using event_binder_type_t = detected_or_t<const default_event_binder, decl_event_binder_type_t, T>;

    template<class EventSource, class EventTarget>
    int run_event_loop(const EventSource& source, EventTarget&& target) noexcept
    {
        using event_binder_type = event_binder_type_t<resource_type_t<std::remove_cvref_t<EventSource>>>;
        auto& target_ref = as_reference(target);
        const auto event_bind_holder = event_binder_type{source}.bind(target_ref);
        private_detail_event_loop::sizes_initialization();
        return private_detail_event_loop::run_event_loop_impl(target_ref);
    }

#elif defined(D_OS_ANDROID)
    namespace private_detail_event_loop
    {
        class message
        {
        public:
            constexpr explicit message(window_handle_t window) noexcept
                : window_{ window }
            {
                D_ASSERT(window_);
            }

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
            bool process(module_handle_t app) const noexcept;

        private:
            window_handle_t window_;
            android_poll_source* source_{ nullptr };
            int events_{ 0 };
        };

        template<class Processor>
        struct message_callbacks_instance
        {
            static_assert(!std::is_reference_v<Processor>);

            static void cmd_callback(module_handle_t app, int32_t cmd) noexcept
            {
                const ui::cmd_event e{ to_cmd_event_style(cmd) };
                do_cmd_event_match(processor_ref(app), e);
            }

            [[nodiscard]]
            static int input_event_callback(module_handle_t app, AInputEvent* input_e) noexcept
            {
                if (const ui::event e { input_e })
                {
                    do_event_match(processor_ref(app), e);
                }

                return 0;
            }

            static Processor& processor_ref(const_module_handle_t app) noexcept
            {
                return *static_cast<Processor*>(user_data(app));;
            }
        };

        template<class Processor>
        int run_event_loop(module_handle_t app, Processor& processor) noexcept
        {
            using message_callbacks_instance_t = message_callbacks_instance<std::remove_reference_t<Processor>>;
            set_user_data(app, as_mutable_pointer(std::addressof(processor)));
            set_cmd_callback(app, message_callbacks_instance_t::cmd_callback);
            set_input_event_callback(app, message_callbacks_instance_t::input_event_callback);

            for (message msg{ app_window_handle(app) }; !msg.poll(do_idle(processor)) || msg.process(app); )
            {}

            return EXIT_SUCCESS;
        }
    }

    template<class EventSource, class T>
    int run_event_loop(const EventSource& source, T&& target) noexcept
    {
        return private_detail_event_loop::run_event_loop
        (
            const_cast<module_handle_t>(static_cast<const_module_handle_t>(source)), 
            as_reference(target)
        );
    }

#endif

    template<class EventSource>
    int run_event_loop(const EventSource& source) noexcept
    {
        constexpr struct {} nop;
        return run_event_loop(source, nop);
    }
}