#pragma once

#include <platform/windows/config.h>

namespace os_windows
{
    struct window_view
    {
        HWND handle_;

        constexpr window_view( HWND handle ) noexcept
            : handle_{ handle }
        {}
    };
}