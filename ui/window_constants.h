#pragma once

#include <px/fwd.h>


namespace ui
{
    enum class show_command
    {
        hide,
        normal,
        minimazed,
        maximazed,
        inactive,
        show,
        restore = 9
    };

    namespace private_detail_window_constants
    {
        using native_px_t = int;

        constexpr auto cw_usedefault = static_cast<native_px_t>(0x80000000);
        constexpr auto px_usedefault = static_cast<pxside_t>(cw_usedefault);
        constexpr pxrectangle rc_usedefault{ px_usedefault, 0_px, px_usedefault, 0_px };
    }

    using private_detail_window_constants::px_usedefault;
    using private_detail_window_constants::rc_usedefault;
}
