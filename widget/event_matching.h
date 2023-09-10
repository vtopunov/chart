#pragma once

#include <widget/window.h>


namespace widget
{
    template<class Event>
    struct widget_event_traits
    {
        using event_result_type = event_result;

        static constexpr bool continue_processing(event_result_type) noexcept
        {
            return true;
        }
    };

    template<>
    struct widget_event_traits<window>
    {
        using event_result_type = window_configation;

        static constexpr bool continue_processing(event_result_type result) noexcept
        {
            return badcfg != result;
        }
    };


    template<class E>
    using event_result_t = typename widget_event_traits<std::remove_cvref_t<E>>::event_result_type;

    template<class Fn, class Arg>
    using event_proccessor_return_t = std::remove_cv_t<
        decltype(std::declval<std::add_lvalue_reference_t<Fn>>()(std::declval<std::add_lvalue_reference_t<Arg>>()))
    >;

    template<class T, class E>
    std::enable_if_t<std::is_same_v<event_proccessor_return_t<T, E>, void>> call_widget_event
    (
        event_result_t<E>&,
        T& function,
        const E& e
    ) noexcept
    {
        function(e);
    }

    template<class T, class E>
    std::enable_if_t<std::is_same_v<event_proccessor_return_t<T, E>, bool>> call_widget_event
    (
        event_result_t<E>& result,
        T& function,
        const E& e
    ) noexcept
    {
        result |= function(e);
    }

    template<class T, class E>
    std::enable_if_t<std::is_enum_v<event_proccessor_return_t<T, E>>> call_widget_event
    (
        event_result_t<E>& result,
        T& function,
        const E& e
    )
    {
        result |= function(e);
    }

    struct no_overloaded_event
    {
        template<class T>
        constexpr no_overloaded_event(const T&) noexcept
        {
            static_assert(std::is_enum_v<event_result_t<T>>);
        }
    };

    struct no_overloaded_event_result
    {
        template<class T>
        constexpr no_overloaded_event_result(const T&) noexcept
        {
            static_assert(std::is_enum_v<T>);
        }
    };

    constexpr void call_widget_event(no_overloaded_event_result, no_overloaded, no_overloaded_event) noexcept
    {}


    template<class Widget, class Event>
    void apply_event(event_result_t<Event>& result, Widget& widget, const Event& e) noexcept
    {
        if (widget_event_traits<Event>::continue_processing(result)) [[likely]]
        {
            call_widget_event(result, widget, e);
            widget.apply([&result, &e] (auto&... widgets) noexcept
            {
                (apply_event(result, widgets, e), ...);
            });
        }
    }
}
