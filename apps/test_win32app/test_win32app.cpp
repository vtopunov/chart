#include <core/debug.h>
#include <ui/window.h>
#include <ui/event_matching.h>
#include <ui/event_processor.h>
#include <ui/event_timer.h>
#include <ui/event_loop.h>

using namespace std::chrono_literals;

namespace
{
    constexpr rect_px_t make_subwindow_rect(size2d_t sizes) noexcept
    {
        return
        {
            vec2_cast<point2d_px_t>( sizes / 4u ),
            vec2_cast<point2d_px_t>( (3u * sizes) / 4u )
        };
    }

    struct main_processor
    {
        static constexpr auto standby_time{ 15s };

        ui::event_timer quit_timer{ ui::create_event_timer(standby_time) };

        ui::event_result_t operator () (const ui::size_event& e) const noexcept
        {
            const auto rc = make_subwindow_rect(e.sizes());

            for ( const auto& children : childrens(e.window()) )
            {
                ui::rect(children, rc);
            }

            return 0L;
        }

        ui::event_result_t operator () (const ui::mouse_move_event& e) noexcept
        {
            output_debug_string("mouse move: {} {}\n", e.x(), e.y());
            quit_timer = set_event_timer(std::move(quit_timer), standby_time);
            D_ASSERT(quit_timer);
            return 0L;
        }

        void operator () (const ui::timer_event& e) const noexcept
        {
            if ( e.timer_id == quit_timer )
            {
                ui::quit();
            }
        }
    };
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int command_show)
{
    ui::window_type_factory type_factory;
    type_factory.module_instance(instance);

    const auto mainwindow =
        ui::window_factory{}
        .type(type_factory.background(ui::stock_brush::dark_gray).create())
        .title(L"test_win32app")
        .create();

    if ( !mainwindow )
    {
        output_debug_string("create window error {}\n", ui::error_code());
        return -1;
    }

    const auto subwindow =
        ui::window_factory{}
        .type(type_factory.background(ui::stock_brush::light_gray).create())
        .parent(mainwindow)
        .rect(make_subwindow_rect(ui::rect(mainwindow).sizes()))
        .create();

    if ( !subwindow )
    {
        output_debug_string("create subwindow error {}\n", ui::error_code());
        return -1;
    }

    ui::show(mainwindow, command_show);

    return ui::run_event_loop(mainwindow, main_processor{});
}

