#pragma once

#include <core/types_algorithm.h>
#include <core/reference_wrapper.h>

#include <ui/event.h>
#include <ui/manipulator.h>

#include <widget/window.h>


namespace widget
{
    template<class EventBase, class... Args>
    class widget_event_base : public EventBase
    {
    public:
        using stdtuple = std::tuple<ref_wrap_if_need_t<Args>...>;
        using decay_ttypes = ttypes<std::decay_t<Args>...>;

        template<class T>
        static constexpr size_t index_element_v = ttypes_index_element_v<std::decay_t<T>, decay_ttypes>;

        using context_ttypes = decay_ttypes;

        template<class... ContextArgs>
        constexpr explicit widget_event_base(const EventBase& e, ref_wrap_if_need_t<Args>... args) noexcept
            : EventBase{ e }
            , tuple_{ args... }
        {}

        template<class T>
        [[nodiscard]] constexpr auto get() const noexcept -> 
            decltype(unorefwrap(std::get<index_element_v<T>>(std::declval<stdtuple&>())))
        {
            return std::get<index_element_v<T>>(tuple_);
        }

    private:
        stdtuple tuple_;
    };

    static_assert(std::is_same_v<ttypes<dummy>, decl_context_t<widget_event_base<dummy, dummy>>>);

    template<class EventBase>
    class widget_event_base<EventBase> : public EventBase
    {
    public:
        using context_ttypes = ttypes<>;

        constexpr explicit widget_event_base(const EventBase& e) noexcept
            : EventBase{ e }
        {}
    };

    static_assert(std::is_same_v<ttypes<>, decl_context_t<widget_event_base<dummy>>>);


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
            return this->template get<widget::window>();
        }

        [[nodiscard]]
        constexpr window_content content() const noexcept
        {
            return window().content();
        }

        [[nodiscard]]
        constexpr pxsizes viewport() const noexcept
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
            return this->template get<ui::gesture>();
        }

        [[nodiscard]]
        constexpr point2re shift() const noexcept
        {
            return gesture().shift();
        }

        [[nodiscard]]
        constexpr size2re scale() const noexcept
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
    class basic_widget_event<EventBase, const window, Args...> : public window_interface_window_event<widget_event_base<EventBase, const window, Args...>>
    {
    public:
        using window_interface_window_event<widget_event_base<EventBase, const window, Args...>>::window_interface_window_event;
    };

    template<class EventBase, class... Args>
    class basic_widget_event<EventBase, const ui::gesture, Args...> : public gesture_interface_widget_event<widget_event_base<EventBase, const ui::gesture, Args...> >
    {
    public:
        using gesture_interface_widget_event<
            widget_event_base<EventBase, const ui::gesture, Args...>
        >::gesture_interface_widget_event;
    };

    template<class EventBase, class... Args>
    class basic_widget_event<EventBase, const window, const ui::gesture, Args...> : public widget_event_gesture_window_interface<
        widget_event_base<EventBase, const window, const ui::gesture, Args...>
    >
    {
    public:
        using widget_event_gesture_window_interface<
            widget_event_base<EventBase, const window, const ui::gesture, Args...>
        >::widget_event_gesture_window_interface;
    };
}