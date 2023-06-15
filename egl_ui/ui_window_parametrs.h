#pragma once

#include <os/fwd.h>

#include <ui/window_constants.h>


namespace egl_ui
{
    struct ui_window_parametrs
    {
        pxrectangle geometry{ ui::rc_usedefault };
        os::module_handle_t module{ nullptr };
    };
}
