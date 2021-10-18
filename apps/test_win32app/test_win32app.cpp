#include <core/zstring_view.h>

#include <core/debug.h>
#include <display/window.h>
#include <display/event_matching.h>
#include <display/event_processor.h>
#include <display/event_timer.h>
#include <display/event_loop.h>

using namespace display;
using namespace std::chrono_literals;

namespace
{
    constexpr pixel_rect_t make_subwindow_rect(pixel_vec2_t sizes) noexcept
    {
        return
        {
            sizes / 4,
            ( 3 * sizes ) / 4
        };
    }

    struct main_processor
    {
        static constexpr auto standby_time{ 15s };

        event_timer quit_timer{ create_event_timer(standby_time) };

        event_result_t operator () (const size_event& e) const noexcept
        {
            const auto rc = make_subwindow_rect(e.sizes());

            for ( const auto& children : childrens(e.window()) )
            {
                display::rect(children, rc);
            }

            return 0L;
        }

        event_result_t operator () (const mouse_move_event& e) noexcept
        {
            output_debug_string("mouse move: {} {}\n", e.x(), e.y());
            quit_timer = set_event_timer(std::move(quit_timer), standby_time);
            D_ASSERT(quit_timer);
            return 0L;
        }

        void operator () (const timer_event& e) const noexcept
        {
            if ( e.timer_id == quit_timer )
            {
                quit();
            }
        }
    };
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int command_show)
{
    window_type_factory type_factory;
    type_factory.module_instance(instance);

    const auto mainwindow =
        window_factory{}
        .type(type_factory.background(stock_brush::dark_gray).create())
        .title(L"test_win32app")
        .create();

    if ( !mainwindow )
    {
        output_debug_string("create window error {}\n", display::last_error_code());
        return -1;
    }

    const auto subwindow =
        window_factory{}
        .type(type_factory.background(stock_brush::light_gray).create())
        .parent(mainwindow)
        .rect(make_subwindow_rect(display::rect(mainwindow).sizes()))
        .create();

    if ( !subwindow )
    {
        output_debug_string("create subwindow error {}\n", display::last_error_code());
        return -1;
    }

    show(mainwindow, command_show);

    return run_event_loop(mainwindow, main_processor{});
}

