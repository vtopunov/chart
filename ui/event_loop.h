#pragma once

#include <os/os_detection.h>

#ifdef D_OS_WINDOWS
#include <core/utility.h>
#endif

#include <ui/app.h>

#ifdef D_OS_WINDOWS
#include <ui/event_processor.h>
#endif

#include <ui/event_matching.h>


namespace ui
{
    using milliseconds_t = std::chrono::milliseconds;

    constexpr auto infinite = milliseconds_t{ D_CONDITIONAL_OS_WINDOWS(0xffffffff, -1) };

    constexpr idle_event idle_event_v{};

    template<class T>
    [[nodiscard]] auto do_idle(T& processor) noexcept
        -> decltype(processor(idle_event_v))
    {
        return processor(idle_event_v);
    }

    [[nodiscard]]
    constexpr milliseconds_t do_idle(no_overloaded) noexcept
    {
        return infinite;
    }


#if defined(D_OS_WINDOWS)
    namespace private_detail_event_loop
    {
        constexpr auto msg_storage_size = 48_uz;
        constexpr uint_t pm_remove{ 1u };

        void message_wait_for(milliseconds_t timeout) noexcept;

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

            for (;;) [[likely]]
            {
                const milliseconds_t timeout{ do_idle(idle_processor) };
                if (timeout > milliseconds_t::zero()) [[unlikely]]
                {
                    message_wait_for(timeout);
                }

                while (PeekMessageW(pmsg, nullptr, 0u, 0u, pm_remove)) [[unlikely]]
                {
                    process_message(pmsg);
                    if (event_style::quit == e_style(pmsg)) [[unlikely]]
                    {
                        return exit_status(pmsg);
                    }
                }
            }
        }
    }


    template<class T>
    int run_event_loop(window_handle_t mainwindow, T&& processor) noexcept
    {
        const auto event_processing = create_event_processor
        (
            mainwindow,
            as_mutable_pointer(std::addressof(processor)),
            event_callback_v<T>
        );

        private_detail_event_loop::sizes_initialization();

        return private_detail_event_loop::run_event_loop_impl(as_reference(processor));
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
            D_FORCE_INLINE bool poll(const ui::milliseconds_t& timeout) noexcept
            {
                return ALooper_pollAll
                (
                    narrow_cast<int>(timeout.count()),
                    nullptr,
                    &events_,
                    (void**)&source_
                ) >= 0;
            }

            enum class process_result
            {
                continue_processing,
                quit
            };

            [[nodiscard]]
            process_result process(module_handle_t app) const noexcept;

        private:
            window_handle_t window_;
            android_poll_source* source_{ nullptr };
            int events_{ 0 };
        };


        class app_manager
        {
        public:
            D_DISABLE_COPY_MOVE(app_manager);

            constexpr app_manager(module_handle_t app) noexcept
                : app_{ app }
            {}

            template<class Processor> [[nodiscard]]
            int run(Processor& processor) const noexcept
            {
                set_processor(processor);

                message msg{ app_window(app_) };

                while(D_LIKELY(true))
                {
                    const milliseconds_t timeout{ do_idle(processor) };

                    if (D_UNLIKELY(msg.poll(timeout)))
                    {
                        if (D_UNLIKELY(message::process_result::quit == msg.process(app_)))
                        {
                            break;
                        }
                    }
                }

                return EXIT_SUCCESS;
            }

            ~app_manager() noexcept;

        private:
            template<class T>
            struct message_callbacks_instance
            {
                static constexpr ui::event_callback_t callback{ ui::event_callback_instance<T>::callback };

                static void cmd_callback(module_handle_t, int32_t) noexcept
                {}

                [[nodiscard]]
                static int input_event_callback(module_handle_t app, AInputEvent* input_e) noexcept
                {
                    if (const ui::event e{ input_e })
                    {
                        callback(user_data(app), e);
                    }

                    return 0;
                }
            };

            template<class Processor>
            void set_processor(Processor& processor) const noexcept
            {
                using message_callbacks_instance_t = message_callbacks_instance<Processor>;
                set_user_data(app_, as_mutable_pointer(std::addressof(processor)));
                set_cmd_callback(app_, message_callbacks_instance_t::cmd_callback);
                set_input_event_callback(app_, message_callbacks_instance_t::input_event_callback);
            }

        private:
            module_handle_t app_;
        };
    }

    template<class T>
    int run_event_loop(module_handle_t app, T&& processor) noexcept
    {
        const private_detail_event_loop::app_manager app_manager{ app };
        return app_manager.run(processor);
    }

#endif
}