#pragma once

#include <egl_ui/fwd.h>


namespace widget
{
    using egl_ui::const_module_handle_t;

    using stretchable_pxrectangle = ::rectangle<npx_t, pxoff_t>;

    class window;
    using windowrefwrap_t = optional_reference_wrapper<const window>;

    struct ex_context_type_enumerator {};

    using noapply_t = std::tuple<>;

    constexpr noapply_t noapply{};

    template<class... Types>
    struct ex_context;

    template<class T>
    using decl_context_tuple_t = typename T::context_tuple_type;

    template<class... Types>
    class common_context;

    template<class EventBase, class... Args>
    class basic_widget_event;

    template<class EventBase, class... Args>
    using widget_event = basic_widget_event<EventBase, windowrefwrap_t, Args...>;

    namespace event_declaration
    {
        constexpr struct initialization_event_base {} initialization_event_base_v;
        constexpr struct viewport_event_base {} viewport_event_base_v;
        constexpr struct redraw_event_base {} redraw_event_base_v;

        template<class... Args>
        using basic_initialization_event = basic_widget_event<initialization_event_base, Args...>;

        template<class... Args>
        using basic_viewport_event = basic_widget_event<viewport_event_base, Args...>;

        template<class... Args>
        using basic_redraw_event = basic_widget_event<redraw_event_base, Args...>;

        template<class... Args>
        using basic_mouse_wheel_event = basic_widget_event<ui::mouse_wheel_event, Args...>;

        template<class... Args>
        using basic_mouse_move_event = basic_widget_event<ui::mouse_move_event, Args...>;

        template<class... Args>
        using basic_mouse_up_event = basic_widget_event<ui::mouse_up_event, Args...>;

        template<class... Args>
        using basic_mouse_double_click_event = basic_widget_event<ui::mouse_double_click_event, Args...>;

        template<class... Args>
        using basic_gesture_event = basic_mouse_move_event<ui::gesture, Args...>;

        template<class... Args>
        using initialization_event = widget_event<initialization_event_base, Args...>;

        template<class... Args>
        using viewport_event = widget_event<viewport_event_base, Args...>;

        template<class... Args>
        using redraw_event = widget_event<redraw_event_base, Args...>;

        template<class... Args>
        using mouse_wheel_event = widget_event<ui::mouse_wheel_event, Args...>;

        template<class... Args>
        using mouse_move_event = widget_event<ui::mouse_move_event, Args...>;

        template<class... Args>
        using mouse_up_event = widget_event<ui::mouse_up_event, Args...>;

        template<class... Args>
        using mouse_double_click_event = widget_event<ui::mouse_double_click_event, Args...>;

        template<class... Args>
        using gesture_event = mouse_move_event<ui::gesture, Args...>;
    }

    using namespace event_declaration;

    using event_result_underlying_t = size_t;

    enum class event_result : event_result_underlying_t
    {
        idle = 0,
        redraw = (1 << 0),
        invalid = numeric_max_v<event_result_underlying_t>
    };

    [[nodiscard]]
    constexpr event_result operator | (event_result left, event_result right) noexcept
    {
        return e_bit_or(left, right);
    }

    [[nodiscard]]
    constexpr event_result operator | (event_result left, bool right) noexcept
    {
        return left | e_bit_if(!right, event_result::invalid);
    }

    [[nodiscard]]
    constexpr event_result operator | (bool left, event_result right) noexcept
    {
        return right | left;
    }

    [[nodiscard]]
    constexpr event_result redraw_if(bool value) noexcept
    {
        return e_bit_if(value, event_result::redraw);
    }
}