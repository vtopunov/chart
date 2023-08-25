#pragma once

#include <widget/window.h>


namespace widget
{
    struct init_event : window 
    {
        using result_type = window_configation;
    };

    struct redraw_event : window 
    {};

    template<class E>
    using decl_event_result_type_t = typename E::result_type;

    template<class E>
    using event_result_type_t = detected_or_t<event_result, decl_event_result_type_t, E>;

    template<class Fn, class Arg>
    using event_proccessor_return_t = std::remove_cv_t<
        decltype(std::declval<std::add_lvalue_reference_t<Fn>>()(std::declval<std::add_lvalue_reference_t<Arg>>()))
    >;

    template<class T, class E>
    std::enable_if_t<std::is_same_v<event_proccessor_return_t<T, E>, void>> call_widget_event
    (
        event_result_type_t<E>&, 
        T& function, 
        const E& e
    ) noexcept
    {
        function(e);
    }

    template<class T, class E>
    std::enable_if_t<std::is_same_v<event_proccessor_return_t<T, E>, bool>> call_widget_event
    (
        event_result_type_t<E>& result,
        T& function,
        const E& e
    ) noexcept
    {
        result |= function(e);
    }

    template<class T, class E>
    std::enable_if_t<std::is_enum_v<event_proccessor_return_t<T, E>>> call_widget_event
    (
        event_result_type_t<E>& result,
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
            static_assert(std::is_enum_v<event_result_type_t<T>>);
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
}
