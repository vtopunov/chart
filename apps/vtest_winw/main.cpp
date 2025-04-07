#include <debug/debug.h>

#include <ui/window.h>
#include <ui/event_loop.h>

using namespace std::chrono_literals;


namespace
{
    constexpr pxrectangle subwindow_geometry(pxsizes mainwindow_sizes) noexcept
    {
        return
        {
            to_point2d(mainwindow_sizes / 4u),
            mainwindow_sizes / 2u
        };
    }

    struct main_processor
    {
        using clock_type = std::chrono::steady_clock;
        using time_point_type = clock_type::time_point;

        static constexpr auto standby_time{ 15s };
        time_point_type last_time{ time_point_type::min() };

        ui::event_result_t operator () (const ui::size_event& e) const noexcept
        {
            const auto rc = subwindow_geometry(e.sizes());

            for ( const auto& children : ui::childrens(e.window()) )
            {
                ui::geometry(children, rc);
            }

            return 0L;
        }

        ui::milliseconds operator () (ui::idle_event) noexcept
        {
            const auto now = clock_type::now();

            if ( (last_time > last_time.min()) && (now - last_time) > standby_time)
            {
                debug("standby timeout: quit by timeout");
                ui::quit();
            }

            last_time = now;

            return standby_time;
        }
    };
}


int app_main(os::module_handle_t app) noexcept
{
    debug("create main window");
    ui::window_builder builder{};
    builder.module(app);

    const auto mainwindow 
        = builder
        .background(ui::stock_brush::dark_gray)
        .title(L"vtest_winw")
        .build();

    if ( !mainwindow )
    {
        e_debug("create window error {}", ui::error_code());
        return EXIT_FAILURE;
    }

    debug("create subwindow");
    const auto subwindow 
        = builder
        .background(ui::stock_brush::light_gray)
        .parent(mainwindow)
        .build();

    if ( !subwindow )
    {
        e_debug("create subwindow error {}", ui::error_code());
        return EXIT_FAILURE;
    }

    debug("show main window");
    ui::show(mainwindow);

    debug("run event loop");
    return ui::run_event_loop(mainwindow, main_processor{});
}

