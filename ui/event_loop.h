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
#if defined(D_OS_WINDOWS)
    namespace private_detail_event_loop
    {
        constexpr auto msg_storage_size = 48_uz;
        constexpr uint_t pm_remove{ 1u };

        void sleep_or_reñeive_message(milliseconds_t timeout) noexcept;

        std::optional<int> process_message(const os::message_t* msg) noexcept;
    }

    template<class T>
    int run_event_loop(window_handle_t mainwindow, T&& processor) noexcept
    {
        using namespace private_detail_event_loop;

        const auto processor_ptr = std::addressof(processor);

        constexpr event_callback_t callback{ event_callback_instance<decltype(processor_ptr)>::callback };

        const auto event_processing = create_event_processor
        (
            mainwindow,
            as_mutable_pointer(processor_ptr),
            callback
        );

        std::byte msg_storage[msg_storage_size]{};
        const auto pmsg = reinterpret_cast<os::message_t*>(std::data(msg_storage));

        for (;;) [[likely]]
        {
            const milliseconds_t timeout{ call_event(processor_ptr, idle_event{}) };
            if (timeout > milliseconds_t::zero()) [[unlikely]]
            {
                sleep_or_reñeive_message(timeout);
            }
            
            while (PeekMessageW(pmsg, nullptr, 0u, 0u, pm_remove)) [[unlikely]]
            {
                const auto exit_status_opt = process_message(pmsg);
                if (exit_status_opt.has_value()) [[unlikely]]
                {
                    return *exit_status_opt;
                }
            }
        }

        return EXIT_SUCCESS;
    }

    inline int run_event_loop(window_handle_t mainwindow) noexcept
    {
        constexpr struct {} nop;
        return run_event_loop(mainwindow, nop);
    };

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

            std::optional<int> process(module_handle_t app) const noexcept;

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
            {
                D_ASSERT(app);
            }

            template<class ProcessorPtr>
            int run(ProcessorPtr processor_ptr) const noexcept
            {
                set_processor(processor_ptr);

                message msg{ app::window(app_) };

                for (;;)
                {
                    const milliseconds_t timeout{ call_event(processor_ptr, idle_event{}) };

                    if (D_UNLIKELY(msg.poll(timeout)))
                    {
                        const auto exit_status_opt = msg.process(app_);
                        if (D_UNLIKELY(exit_status_opt.has_value()))
                        {
                            return *exit_status_opt;
                        }
                    }
                }

                return EXIT_SUCCESS;
            }
            
            ~app_manager() noexcept
            {
                app::set_user_data(app_, nullptr);
                app::quit(app_);
            }

        private:
            template<class T>
            struct message_callbacks_instance
            {
                static constexpr ui::event_callback_t callback{ ui::event_callback_instance<T>::callback };

                static void cmd_callback(module_handle_t app, int32_t cmd) noexcept
                {
                    const ui::event e { underlying_cast<ui::event_style>(cmd) };
                    callback(app::user_data(app), e);
                }

                static int input_event_callback(module_handle_t app, AInputEvent* input_e) noexcept
                {
                    const ui::input_event e{ input_e };
                    callback(app::user_data(app), e);
                    return 0;
                }
            };

            template<class ProcessorPtr>
            void set_processor(ProcessorPtr processor_ptr) const noexcept
            {
                D_ASSERT(processor_ptr);

                using message_callbacks_instance_t = message_callbacks_instance<ProcessorPtr>;
                app::set_user_data(app_, as_mutable_pointer(processor_ptr));
                app::set_cmd_callback(app_, message_callbacks_instance_t::cmd_callback);
                app::set_input_event_callback(app_, message_callbacks_instance_t::input_event_callback);
            }

        private:
            module_handle_t app_;
        };
    }

    template<class T>
    int run_event_loop(module_handle_t app, T&& processor) noexcept
    {
        const private_detail_event_loop::app_manager app_manager{ app };
        return app_manager.run(std::addressof(processor));
    }

    inline int run_event_loop(module_handle_t app) noexcept
    {
        constexpr struct {} nop;
        return run_event_loop(app, nop);
    };

#endif
}