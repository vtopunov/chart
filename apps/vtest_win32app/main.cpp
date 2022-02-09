#include <os/debug.h>

#include <ui/window.h>
#include <ui/timer.h>
#include <ui/event_loop.h>

using namespace std::chrono_literals;

namespace
{
    constexpr px::rect make_subwindow_rect(px::size2d sizes) noexcept
    {
        return
        {
            narrow2d_cast<px::point2d>( sizes / 4u ),
            narrow2d_cast<px::point2d>( (3u * sizes) / 4u )
        };
    }

    struct main_processor
    {
        static constexpr auto standby_time{ 15s };

        ui::timer quit_timer{ ui::create_timer(standby_time) };

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
            debug("mouse move: {} {}", e.x(), e.y());
            quit_timer = set_timer(std::move(quit_timer), standby_time);
            D_ASSERT(quit_timer);
            return 0L;
        }

        void operator () (const ui::timer_event& e) const noexcept
        {
            if ( e.is(quit_timer) )
            {
                ui::quit();
            }
        }
    };
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int command_show)
{
    debug("create main window");
    ui::type_window_factory type_factory;
    type_factory.module_instance(instance);

    const auto mainwindow =
        ui::window_factory{}
        .type(type_factory.background(ui::stock_brush::dark_gray).create())
        .title(L"vtest_win32app")
        .create();

    if ( !mainwindow )
    {
        e_debug("create window error {}", ui::error_code());
        return EXIT_FAILURE;
    }


    debug("create subwindow");
    const auto subwindow =
        ui::window_factory{}
        .type(type_factory.background(ui::stock_brush::light_gray).create())
        .parent(mainwindow)
        .rect(make_subwindow_rect(ui::rect(mainwindow).sizes()))
        .create();

    if ( !subwindow )
    {
        e_debug("create subwindow error {}", ui::error_code());
        return EXIT_FAILURE;
    }

    debug("show main window");
    ui::show(mainwindow, command_show);

    debug("run event loop");
    return ui::run_event_loop(mainwindow, main_processor{});
}

