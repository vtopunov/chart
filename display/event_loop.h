#pragma once

#include <core/utility.h>

#include <display/defs.h>
#include <display/event_processor.h>
#include <display/event_matching.h>

namespace display
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
#pragma warning(push)
#pragma warning(disable : 26472) // Don't use static_cast for arithmetic conversions
            return static_cast<int>( msg.wParam );
#pragma warning(pop)
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

        const auto event_processing = event_processor::bind
        (
            mainwindow,
            processor_ptr,
            callback
        );

        for ( native_event msg{};;)
        {
            if ( call_event(processor_ptr, peek_event{}) )
            {
                while ( msg.try_receive() )
                {
                    msg.translate_and_dispatch();
                    if ( msg.is_quit() )
                    {
                        return msg.exit_status();
                    }
                }

                call_event(processor_ptr, idle_event{});
            }
            else
            {
                if ( msg.receive() )
                {
                    call_event(processor_ptr, msg.msg);
                    msg.translate_and_dispatch();
                }
                else
                {
                    break;
                }
            }
        }

        return 0;
    }
}
