#pragma once

#include <optional>

#include <core/functional.h>

#include <os/fwd.h>
#include <px/fwd.h>


namespace ui
{
#ifdef D_OS_WINDOWS
    using os::gdi_object_handle_t;
    using os::brush_handle_t;
    using os::const_brush_handle_t;
#endif
    using os::window_handle_t;
    using os::module_handle_t;
    using os::const_module_handle_t;


#ifdef D_OS_WINDOWS
    using os::uint_t;
    using os::dword_t;
    using os::word_t;
    using os::word_parameter_t;
    using os::long_parameter_t;

    using os::wndproc_t;
    using os::def_window_proc;
#endif

#ifdef D_OS_ANDROID
    using os::sensor_manager_handle_t;
    using os::sensor_event_queue_handle_t;
#endif

    class event;
    class pointer_event;
    class mouse_event;

    using event_result_t = ptrdiff_t;
    using event_result_opt_t = D_CONDITIONAL_OS_WINDOWS(std::optional<event_result_t>, std::nullopt_t);
    using event_callback_t = unique_function<event_result_opt_t (const event&)>;

#ifdef D_OS_WINDOWS
    static_assert(std::is_same_v<event_result_t, os::long_result_t>);
#endif

    enum class event_style : D_CONDITIONAL_OS_WINDOWS(uint_t, int32_t)
    {
        null = D_CONDITIONAL_OS_WINDOWS(0x0000, -1),

#ifdef D_OS_WINDOWS
        size = 0x0005,
        quit = 0x0012,
        mouse_double_click = 0x0203,
        mouse_wheel = 0x020A,
#endif

        mouse_move = D_CONDITIONAL_OS_WINDOWS(0x0200, 0x02),
        mouse_down = D_CONDITIONAL_OS_WINDOWS(0x0201, 0x00),
        mouse_up = D_CONDITIONAL_OS_WINDOWS(0x0202, 0x01),
    };

    template<event_style>
    class specialized_event;

#ifdef D_OS_WINDOWS
    using size_event = specialized_event<event_style::size>;
    using mouse_wheel_event = specialized_event<event_style::mouse_wheel>;
    using mouse_double_click_event = specialized_event<event_style::mouse_double_click>;
#endif

    using mouse_move_event = specialized_event<event_style::mouse_move>;
    using mouse_down_event = specialized_event<event_style::mouse_down>;
    using mouse_up_event = specialized_event<event_style::mouse_up>;
    struct idle_event {};

#ifdef D_OS_ANDROID
    enum class cmd_event_style : int32_t
    {
        redraw_needed = 0x04,
        content_rect_changed = 0x05
    };

    class cmd_event;

    template<cmd_event_style>
    class specialized_cmd_event;

    using redraw_needed_event = specialized_cmd_event<cmd_event_style::redraw_needed>;
    using content_rect_changed_event = specialized_cmd_event<cmd_event_style::content_rect_changed>;

#else
    struct redraw_needed_event {};

#endif

    constexpr pxsize2d no_window_sizes{ 0_px, 0_px };

    constexpr bool window_sizes_is_valid(pxsize2d sizes) noexcept
    {
        static_assert(std::is_unsigned_v<decltype(sizes.height())>);
        return !!sizes.height();
    }

    static_assert(!window_sizes_is_valid(no_window_sizes));
}
