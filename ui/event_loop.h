#pragma once

#include <core/utility.h>

#include <ui/event_processor.h>
#include <ui/event_matching.h>

namespace ui
{
    struct native_event
    {
        [[nodiscard]]
        bool receive() noexcept
        {
            return !!GetMessageW(&msg, nullptr, 0u, 0u);
        }

        [[nodiscard]]
        bool try_receive() noexcept
        {
            return !!PeekMessageW(&msg, nullptr, 0u, 0u, PM_REMOVE);
        }

        void translate_and_dispatch() const noexcept
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        [[nodiscard]]
        constexpr bool is_quit() const noexcept
        {
            return msg.message == WM_QUIT;
        }

        [[nodiscard]]
        constexpr int exit_status() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
            return static_cast<int>(msg.wParam);
            D_WARNING_POP;
        }

        MSG msg;
    };

    template<class T>
    int run_event_loop(window_resource mainwindow, T&& processor) noexcept
    {
        const auto processor_ptr = std::addressof(as_mutable(processor));

        constexpr event_callback_t callback
        {
            event_callback_instance<std::remove_pointer_t<std::remove_const_t<decltype(processor_ptr)>>>::callback
        };

        const auto event_processing = create_event_processor
        (
            mainwindow,
            processor_ptr,
            callback
        );

        const auto translate_and_dispatch = [processor_ptr](const native_event& msg) noexcept
        {
            call_event(processor_ptr, msg.msg);
            msg.translate_and_dispatch();
        };

        for (native_event msg{};;)
        {
            if (call_event(processor_ptr, idle_event{}))
            {
                while (msg.try_receive())
                {
                    translate_and_dispatch(msg);

                    if (msg.is_quit())
                    {
                        return msg.exit_status();
                    }
                }
            }
            else
            {
                if (msg.receive())
                {
                    translate_and_dispatch(msg);
                }
                else
                {
                    break;
                }
            }
        }

        return EXIT_SUCCESS;
    }

    inline int run_event_loop(window_resource mainwindow) noexcept
    {
        constexpr struct {} nop;
        return run_event_loop(mainwindow, nop);
    };
}
