#pragma once

#include <compare>

#include <core/utility.h>
#include <core/rect.h>
#include <platform/windows/config.h>

namespace os_windows
{
    class event;

    using event_result_t = LRESULT;
    using window_handle_t = HWND;
    using const_window_handle_t = add_const_to_pointer_t<window_handle_t>;

    using pixel_t = int;
    using point_t = point<pixel_t>;
    using rect_size_t = rect_size<pixel_t>;
    using rect_t = rect<pixel_t>;

    inline constexpr auto use_default_px = narrow_cast<pixel_t>(CW_USEDEFAULT);
    inline constexpr auto use_default_vec = fill_vec(use_default_px);
    inline constexpr point_t use_default_position{ use_default_vec };
    inline constexpr rect_size_t use_default_size{ use_default_vec };

    struct window_view
    {
        window_handle_t handle;
        size_t id;

        constexpr auto operator<=>(const window_view&) const noexcept = default;
    };

    inline constexpr window_view null_window{ nullptr, 0u };
}