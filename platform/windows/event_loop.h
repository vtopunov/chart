#pragma once

#include <chrono>

#include <core/vec.h>

#include <platform/windows/defs.h>

namespace os_windows
{
    class native_event
    {
    public:
        bool receive() noexcept
        {
            return !!GetMessageW(&msg, nullptr, 0u, 0u);
        }

        bool try_receive() noexcept
        {
            return !!PeekMessageW(&msg, nullptr, 0u, 0u, PM_REMOVE);
        }

        void translate_and_dispatch() const noexcept
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        constexpr bool is_quit() const noexcept
        {
            return msg.message == WM_QUIT;
        }

        constexpr int exit_status() const noexcept
        {
#pragma warning(push)
#pragma warning(disable : 26472) // Don't use static_cast for arithmetic conversions
            return static_cast<int>( msg.wParam );
#pragma warning(pop)
        }

    private:
        MSG msg{};
    };

    struct peek_event
    {};

    struct idle_event
    {};

    struct default_event_loop_processor
    {
        constexpr bool operator () (const peek_event&) const noexcept
        {
            return false;
        }

        constexpr void operator () (const idle_event&) const noexcept
        {
        }
    };

    template<class Function>
    int run_event_loop(Function&& processor) noexcept
    {
        for (native_event msg;;)
        {
            if (processor(peek_event{}))
            {
                while(msg.try_receive())
                {
                    msg.translate_and_dispatch();
                    if (msg.is_quit())
                    {
                        return msg.exit_status();
                    }
                }
                
                processor(idle_event{});
            }
            else
            {
                if (msg.receive())
                {
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

    inline int run_event_loop() noexcept
    {
        return run_event_loop(default_event_loop_processor{});
    }
}
