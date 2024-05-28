#pragma once

#include <optional>

#include <os/fwd.h>
#include <px/fwd.h>


namespace ui
{
    using px::no_sizes;

    using os::window_handle_t;
    using os::module_handle_t;
    using os::const_module_handle_t;
    using os::uint_t;
    using os::dword_t;
    using os::word_t;
    using os::word_parameter_t;
    using os::long_parameter_t;

#ifdef D_OS_WINDOWS
    using os::gdi_object_handle_t;
    using os::brush_handle_t;
    using os::const_brush_handle_t;
    using os::wndproc_t;
    using os::def_window_proc;

#endif

#ifdef D_OS_ANDROID
    using os::sensor_manager_handle_t;
    using os::sensor_event_queue_handle_t;
    struct dummy_wndproc {};
    using wndproc_t = dummy_wndproc;
    constexpr wndproc_t def_window_proc{};

#endif

    class event;
    class pointer_event;
    class mouse_event;

    using event_result_t = ptrdiff_t;
    using event_result_opt_t = D_CONDITIONAL_OS_WINDOWS(std::optional<event_result_t>, std::nullopt_t);

    static_assert(std::is_same_v<event_result_t, os::long_result_t>);

    using event_style_underlying_t = D_CONDITIONAL_OS_WINDOWS(uint_t, int32_t);

    enum class event_style : event_style_underlying_t
    {
        null = D_CONDITIONAL_OS_WINDOWS(0x0000, 0xff),

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

#else
    class size_event;
    class mouse_wheel_event;
    class mouse_double_click_event;

#endif

    using mouse_move_event = specialized_event<event_style::mouse_move>;
    using mouse_down_event = specialized_event<event_style::mouse_down>;
    using mouse_up_event = specialized_event<event_style::mouse_up>;
    struct idle_event {};

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


    struct default_event_binder;

    template<class T>
    using decl_event_binder_type_t = typename T::event_binder_type;

    template<class T>
    using event_binder_type_t = std::add_const_t<detected_or_t<default_event_binder, decl_event_binder_type_t, T>>;


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
        using native_npx_t = int;

        constexpr auto cw_usedefault = static_cast<native_npx_t>(0x80000000);
        constexpr auto px_usedefault = static_cast<npx_t>(cw_usedefault);
        constexpr pxrectangle rc_usedefault{ px_usedefault, 0_npx, px_usedefault, 0_npx };
    }

    using private_detail_window_constants::px_usedefault;
    using private_detail_window_constants::rc_usedefault;

    namespace manipulator
    {
        struct gesture;
    }

    using manipulator::gesture;
}
