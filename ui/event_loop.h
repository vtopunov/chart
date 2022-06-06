#pragma once

#include <core/utility.h>

#include <ui/event_processor.h>
#include <ui/event_matching.h>

namespace ui
{
    inline void sleep_or_reñeive_event(milliseconds_t timeout) noexcept
    {
        if (timeout > timeout.zero()) [[unlikely]]
        {
            MsgWaitForMultipleObjectsEx
            (
                0u, 
                nullptr, 
                narrow_cast<DWORD>(std::min(infinite, timeout).count()), 
                QS_ALLEVENTS, 
                MWMO_ALERTABLE
            );
        }
    }

    struct native_event
    {
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
    int run_event_loop(window_handle_t mainwindow, T&& processor) noexcept
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

        for (native_event msg{};;) [[likely]]
        {
            sleep_or_reñeive_event(call_event(processor_ptr, idle_event{}));

            while (msg.try_receive()) [[unlikely]]
            {
                msg.translate_and_dispatch();

                if (msg.is_quit()) [[unlikely]]
                {
                    return msg.exit_status();
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
}
