#pragma once

#include <compare>

#include <core/defs.h>
#include <core/rect.h>
#include <platform/windows/config.h>

namespace os_windows
{
    using pixel_t = int;
    using vec_t = vec<pixel_t>;
    using point_t = point<pixel_t>;
    using rect_size_t = rect_size<pixel_t>;
    using rect_t = rect<pixel_t>;

    inline constexpr auto use_default_px = narrow_cast<pixel_t>( CW_USEDEFAULT );
    inline constexpr auto use_default_vec = fill_vec(use_default_px);
    inline constexpr auto use_default_position = make_point(use_default_vec);
    inline constexpr auto use_default_size = make_rect_size(use_default_vec);

    class event;

    using event_result_t = LRESULT;

    using window_handle_t = HWND;
    using const_window_handle_t = add_const_to_pointer_t<window_handle_t>;

    struct window_view
    {
        window_handle_t handle;
        size_t id;

        constexpr auto operator<=>(const window_view&) const noexcept = default;
    };

    inline constexpr window_view null_window{ nullptr, 0u };
}