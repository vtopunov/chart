#include <debug/debug.h>

#include <ui/window.h>
#include <ui/event_loop.h>


int app_main(os::module_handle_t app) noexcept
{
    debug("create main window");
    const auto mainwindow 
        = ui::window_builder{}
        .module(app)
        .build();

    if ( !mainwindow )
    {
        e_debug("create window error {}", ui::error_code());
        return EXIT_FAILURE;
    }

    debug("show main window");
    ui::show(mainwindow);

    debug("run event loop");
    return ui::run_event_loop(mainwindow);
}

