
#include <ui/window.h>
#include <ui/event_loop.h>
#include <ui/debug.h>


int main() noexcept
{
    debug("create main window");
    const auto mainwindow  = ui::window_builder{}.build();
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

