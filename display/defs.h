#pragma once

#include <compare>
#include <optional>

#include <core/null.h>
#include <core/rect.h>
#include <core/os.h>

namespace display
{
    class event;

    using event_result_t = LRESULT;
    using window_handle_t = HWND;

    using pixel_t = int;
    using pixel_vec2_t = vec2<pixel_t>;
    using pixel_rect_t = rect<pixel_t>;

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

    using event_callback_t = std::optional<event_result_t>(*)(void*, const event&);
}