#pragma once

#include <core/null.h>
#include <core/os.h>

namespace ui
{
    using window_handle_t = HWND;

    struct window_resource
    {
        window_handle_t handle;

        [[nodiscard]]
        constexpr auto operator<=>(const window_resource&) const noexcept = default;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return !!handle;
        }
    };

    using nullwindow_t = null_t<window_resource>;
    inline constexpr nullwindow_t nullwindow{};
}