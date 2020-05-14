#pragma once

#include <platform/windows/config.h>

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
            return static_cast<int>(msg.wParam);
#pragma warning(pop)
        }

    private:
        MSG msg{};
    };

    inline int run_event_loop() noexcept
    {
        native_event msg;

        while (msg.receive())
        {
            msg.translate_and_dispatch();
        }

        return msg.exit_status();
    }

    template<class Function>
    int run_event_loop(Function&& on_idle) noexcept
    {
        native_event msg;
        bool is_idle = true;

        for (;;)
        {
            if (is_idle)
            {
                if (msg.try_receive())
                {
                    msg.translate_and_dispatch();

                    if (msg.is_quit())
                    {
                        break;
                    }
                }
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

            
            is_idle = on_idle();
        }

        return msg.exit_status();
    }
}
