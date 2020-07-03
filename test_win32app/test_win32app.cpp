#include <core/zstring_view.h>

#include <platform/windows/debug.h>
#include <platform/windows/window.h>
#include <platform/windows/event_matching.h>
#include <platform/windows/event_timer.h>
#include <platform/windows/event_loop.h>

using namespace os_windows;
using namespace std::chrono_literals;

namespace
{
    namespace debug
    {
        namespace strings
        {
            constexpr zstring_view alert{ "alert" };
            constexpr zstring_view destroy{ "destroy" };
            constexpr zstring_view timer1_callback1{ "timer1_callback1" };
            constexpr zstring_view timer1_callback2{ "timer1_callback2" };
            constexpr zstring_view timer2_callback1{ "timer2_callback1" };
            constexpr zstring_view timer3_callback1{ "timer3_callback1" };
            constexpr zstring_view manual_exit{ "manual_exit\n" };
        }

        void log(zstring_view message) noexcept
        {
            class logger
            {
            public:
                void output(zstring_view message)
                {
                    messages_.emplace_back(message);
                    output_debug_string(message.c_str());
                }

                ~logger() noexcept
                {
                    constexpr auto test_log = [] (span<std::string> log) noexcept
                    {
                        using namespace strings;

                        for ( const auto& message : log )
                        {
                            if ( message == manual_exit )
                            {
                                return;
                            }
                        }

                        constexpr std::string_view test_messages[] =
                        {
                            alert, timer1_callback1,
                            destroy, timer1_callback1,
                            alert, timer1_callback2,
                            alert, timer2_callback1,
                            destroy, timer2_callback1,
                            alert, timer3_callback1,
                            destroy, timer3_callback1,
                            destroy, timer1_callback2
                        };
  
                        constexpr auto test_size = std::size(test_messages);

                        D_ASSERT(std::size(log) == test_size);

                        for ( size_t i = 0; i < test_size; ++i )
                        {
                            D_ASSERT(log[i] == test_messages[i]);
                        }
                    };

                    test_log(messages_);
                }

            private:
                std::vector<std::string> messages_;
            };

            static logger instance;
            instance.output(message);
        }

        struct dtor_view
        {
            zstring_view debug_message;

            void notify(zstring_view string) const noexcept
            {
                log(string);
                output_debug_string(" ");
                log(debug_message);
                output_debug_string("\n");
            }
        };

        void close(dtor_view dtor) noexcept
        {
            dtor.notify(strings::destroy);
        }

        class dtor
        {
        public:
            explicit dtor(zstring_view message) noexcept
                : handle_{ make_shared_handle<dtor_view>(message) }
            {}

            void alert() const noexcept
            {
                handle_->notify(strings::alert);
            }

        private:
            shared_handle<dtor_view> handle_;
        };
    }

    void quit() noexcept
    {
        PostQuitMessage(0);
    }

    constexpr rect_t make_subwindow_rect(rect_size_t window_size) noexcept
    {
        const auto point = window_size.to_point();
        return 
        {
            point / 4,
            ( 3 * point ) / 4
        };
    }

    constexpr struct
    {
        event_result_t operator () (const close_event& e) const noexcept
        {
            if ( MessageBoxW(e.window().handle, L"Вы хотите выйти из приложения?", L"Выход", MB_YESNO) == IDYES )
            {
                debug::log(debug::strings::manual_exit);
                quit();
            }

            return 0L;
        }

        event_result_t operator () (const size_event& e) const noexcept
        {
            const auto rect = make_subwindow_rect(rect_size_t{e.width(), e.height()});

            for ( const auto children : childrens(e.window()) )
            {
                SetWindowPos
                (
                    children.handle, nullptr,
                    rect.x0(), rect.y0(),
                    rect.width(), rect.height(),
                    SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_SHOWWINDOW
                );
            }

            return 0L;
        }

        event_result_t operator () (const mouse_move_event& e) const noexcept
        {
            output_debug_string("mouse move: {} {}\n", e.x(), e.y());
            return 0L;
        }
    } event_processor;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    const auto main_window =
        window_factory{}
        .title(L"test_win32app")
        .type
        (
             window_type_factory{}
            .module_address(hInstance)
            .name(L"test_win32wnd")
            .background(stock_brush::dark_gray)
            .create()
        )
        .create();

    if ( !main_window )
    {
        output_debug_string("create window error {}", GetLastError());
        return -1;
    }

    show(main_window, nCmdShow);

    const auto subwindow =
        window_factory{}
        .parent(main_window)
        .type
        (
            window_type_factory{}
            .module_address(hInstance)
            .name(L"test_win32subwnd")
            .background(stock_brush::light_gray)
            .create()
        )
        .rect(make_subwindow_rect(client_rect(main_window).size()))
        .create();

    if ( !subwindow )
    {
        output_debug_string("create subwindow error {}", GetLastError());
        return -1;
    }

    const auto process_owner = attach_event_processor(main_window, event_match(event_processor));

    const auto timer = event_timer::create_timer(main_window, 5s,
        [
            dtor11 = debug::dtor{ debug::strings::timer1_callback1 }
        ]( event_timer_controller& timer1_manip1 ) noexcept
    {
        dtor11.alert();
        timer1_manip1.restart(3s);

        timer1_manip1.replace_callback(
            [
                timer2 = safe_event_processor{},
                timer3 = safe_event_processor{},
                dtor12 = debug::dtor{ debug::strings::timer1_callback2 }
            ]( event_timer_controller& timer1_manip2 ) mutable noexcept
        {
            if ( timer2 || timer3 )
            {
                return;
            }

            dtor12.alert();

            timer2 = event_timer::create_timer(timer1_manip2.window(), 2s,
                [
                    &timer3,
                    dtor21 = debug::dtor{ debug::strings::timer2_callback1 }
                ] ( event_timer_controller& timer2_manip ) noexcept
            {
                dtor21.alert();

                const auto window = timer2_manip.window();
                timer2_manip.close();

                timer3 = event_timer::create_timer(window, 1s,
                    [
                        first_call = true,
                        dtor31 = debug::dtor{ debug::strings::timer3_callback1 }
                    ]( event_timer_controller& timer3_manip ) mutable noexcept
                {
                    if ( std::exchange(first_call, false) )
                    {
                        dtor31.alert();
                    }
                    quit();
                });
            });
        });
    });

    return run_event_loop();
}

