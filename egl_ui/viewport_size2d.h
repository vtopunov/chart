#pragma once

#include <ui/fwd.h>


namespace egl_ui
{
    struct viewport_size2d : pxsize2d
    {
        constexpr explicit operator bool() const noexcept
        {
            return ui::window_sizes_is_valid(*this);
        }
    };

    static constexpr viewport_size2d no_viewport{ ui::no_window_sizes };
    static_assert(!no_viewport);
}

using egl_ui::viewport_size2d;
