// test_win32app.cpp : Defines the entry point for the application.
//

#include <platform/windows/window.h>
#include <platform/windows/event.h>
#include <platform/windows/event_handler.h>
#include <platform/windows/event_matching.h>
#include <platform/windows/event_timer.h>

using namespace os_windows;
using namespace std::chrono_literals;

namespace
{
    int output_error_code() noexcept
    {
        const auto error_code = GetLastError();
        output_debug_string("error code: %lu", error_code);
        D_CHECK(!"win32 error");
        return (error_code) ? static_cast<int>(error_code) : -1;
    }

    struct dtor_debug
    {
        using string_literal = const char*;

        dtor_debug(string_literal debug_message) noexcept
            : handle_{ { debug_message } }
        {}

        struct handle
        {
            string_literal debug_message{ nullptr };

            void close() noexcept
            {
                if (const auto self = std::exchange(*this, {}); self.debug_message)
                {
                    output_debug_string(self.debug_message);
                }
            }
        };

        shared_handle<handle> handle_;
    };

    void quit() noexcept
    {
        PostQuitMessage(0);
    }

    constexpr struct
    {
        event_result operator () (const close_event& e) const noexcept
        {
            if (MessageBoxW(e.window().handle, L"Вы хотите выйти из приложения?", L"Выход", MB_YESNO) == IDYES)
            {
                quit();
            }

            return accept_event_result;
        }

        event_result operator () (const mouse_move_event& e) const noexcept
        {
            output_debug_string("mouse move: %d %d\n", e.x(), e.y());
            return accept_event_result;
        }
    } event_handlers;

}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    const auto window = create_window
    (
        window_info{}
        .title(L"test_win32app")
        .type
        (
            register_window_type
            (
                window_type_info{}
                .module_address(hInstance)
                .name(L"test_win32wnd")
            )
        )
    );

    if (!window)
    {
        return output_error_code();
    }

    window->show(nCmdShow);
    window->update();

    const auto event_handler 
        = register_event_handler(window, event_match(event_handlers));

    const auto timer = register_timer(window, 5s, 
        [
            dtor11 = dtor_debug{ "destroy timer callback1\n" }
        ](timer_controller& timer_manip) noexcept
    {
        output_debug_string("timer callback1\n");
        D_CHECK(timer_manip.restart(3s));

        timer_manip.replace_callback(
            [
                timer2 = safe_event_handler{}, 
                timer3 = safe_event_handler{}, 
                dtor12 = dtor_debug{ "destroy timer callback2\n" }
            ](timer_controller& timer_manip2) mutable noexcept
        {
            output_debug_string("timer callback2\n");

            if (timer2 || timer3)
            {
                D_CHECK(!"timeout");
                quit();
                return;
            }
            D_CHECK(timer_manip2.restart(7s));

            timer2 = register_timer(timer_manip2.window(), 2s, 
                [
                    &timer3,
                    dtor21 = dtor_debug{ "destroy timer2 callback1\n" }
                ] (timer_controller& timer2_manip) noexcept
            {
                output_debug_string("timer2 callback1\n");
                const auto window = timer2_manip.window();
                timer2_manip.close();
                timer3 = register_timer(window, 1s, 
                    [
                        dtor31 = dtor_debug{ "destroy timer3 callback1\n" }
                    ](timer_controller&) noexcept
                {
                    output_debug_string("timer3 callback1\n");
                    quit();
                });
            });
        });
    });

    MSG msg{};

    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

