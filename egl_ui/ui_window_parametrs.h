#pragma once

#include <os/fwd.h>

#include <ui/window_constants.h>


namespace egl_ui
{
    struct ui_window_parametrs
    {
#ifdef D_OS_WINDOWS
        pxrectangle geometry{ ui::rc_usedefault };
#endif

        os::module_handle_t module{ nullptr };
    };
}
