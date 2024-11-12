#pragma once

#include <core/types_algorithm.h>
#include <core/functional.h>

#include <ui/event.h>
#include <ui/manipulator.h>

#include <widget/window.h>


namespace widget
{
    namespace helpers
    {
        template<class T>
        constexpr auto is_nothrow_copiable_v = std::conjunction_v<
            std::is_nothrow_copy_constructible<T>,
            std::is_nothrow_copy_assignable<T>
        >;

        template<class T>
        using cref_wrap_if_need_t = std::conditional_t<
            is_nothrow_copiable_v<T>, T,
            std::reference_wrapper<std::add_const_t<T>>
        >;

        template<class T>
        using cref_if_need_t = std::conditional_t<
            is_nothrow_copiable_v<T>, std::add_const_t<T>,
            std::add_lvalue_reference_t<std::add_const_t<T>>
        >;
    }

    template<class EventBase, class... Args>
    class widget_event_base : public EventBase
    {
    private:
        using cref_wrap_tuple_type = std::tuple<helpers::cref_wrap_if_need_t<Args>...>;

    public:
        using context_tuple_type = std::tuple<Args...>;

        constexpr explicit widget_event_base(const EventBase& e, const Args&... args) noexcept
            : EventBase{ e }
            , tuple_{ args... }
        {}

        template<class T>
        [[nodiscard]] constexpr helpers::cref_if_need_t<T> get() const noexcept
        {
            return std::get<helpers::cref_wrap_if_need_t<T>>(tuple_);
        }

    protected:
        [[nodiscard]]
        constexpr const cref_wrap_tuple_type& tuple() const noexcept
        {
            return tuple_;
        }

    private:
        cref_wrap_tuple_type tuple_{};
    };

    template<class EventBase>
    class widget_event_base<EventBase> : public EventBase
    {
    public:
        using context_tuple_type = std::tuple<>;

        constexpr explicit widget_event_base(const EventBase& e) noexcept
            : EventBase{ e }

        {}
    };

    template<class EventBase, class... Args>
    class basic_widget_event : public widget_event_base<EventBase, Args...>
    {
    public:
        using widget_event_base<EventBase, Args...>::widget_event_base;
    };

    template<class Base>
    struct window_interface_window_event : Base
    {
        using Base::Base;

        [[nodiscard]]
        constexpr const window& window() const noexcept
        {
            return std::get<windowrefwrap_t>(this->tuple());
        }

        [[nodiscard]]
        constexpr const_module_handle_t app() const noexcept
        {
            return window().module();
        }

        [[nodiscard]]
        constexpr window_content content() const noexcept
        {
            return window().content();
        }

        [[nodiscard]]
        constexpr pxsize2d viewport() const noexcept
        {
            return window().viewport();
        }
    };

    template<class Base>
    struct gesture_interface_widget_event : Base
    {
        using Base::Base;

        [[nodiscard]]
        constexpr const ui::gesture& gesture() const noexcept
        {
            return std::get<ui::gesture>(this->tuple());
        }

        [[nodiscard]]
        constexpr px::real_point2d shift() const noexcept
        {
            return gesture().shift();
        }

        [[nodiscard]]
        constexpr px::real_size2d scale() const noexcept
        {
            return gesture().scale();
        }

        template<class T>
        [[nodiscard]] constexpr auto transformation(const T& v) const noexcept -> decltype(gesture().transformation(v))
        {
            return gesture().transformation(v);
        }

        template<class T>
        [[nodiscard]] constexpr T transformation_as(const T& v) const noexcept
        {
            return gesture().transformation_as(v);
        }
    };

    template<class Base>
    using widget_event_gesture_window_interface = gesture_interface_widget_event<window_interface_window_event<Base>>;

    template<class EventBase, class... Args>
    class basic_widget_event<EventBase, windowrefwrap_t, Args...> : public window_interface_window_event<widget_event_base<EventBase, windowrefwrap_t, Args...>>
    {
    public:
        using window_interface_window_event<widget_event_base<EventBase, windowrefwrap_t, Args...>>::window_interface_window_event;
    };

    template<class EventBase, class... Args>
    class basic_widget_event<EventBase, ui::gesture, Args...> : public gesture_interface_widget_event<widget_event_base<EventBase, ui::gesture, Args...> >
    {
    public:
        using gesture_interface_widget_event<
            widget_event_base<EventBase, ui::gesture, Args...>
        >::gesture_interface_widget_event;
    };

    template<class EventBase, class... Args>
    class basic_widget_event<EventBase, windowrefwrap_t, ui::gesture, Args...> : public widget_event_gesture_window_interface<
        widget_event_base<EventBase, windowrefwrap_t, ui::gesture, Args...>
    >
    {
    public:
        using widget_event_gesture_window_interface<
            widget_event_base<EventBase, windowrefwrap_t, ui::gesture, Args...>
        >::widget_event_gesture_window_interface;
    };
}