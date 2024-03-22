#pragma once

#include <core/tuple_algorithm.h>
#include <core/functional.h>

#include <ui/event.h>

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
        constexpr helpers::cref_if_need_t<T> get() const noexcept
        {
            return std::get<helpers::cref_wrap_if_need_t<T>>(tuple_);
        }

    protected:
        const cref_wrap_tuple_type& tuple() const noexcept
        {
            return tuple_;
        }

    private:
        cref_wrap_tuple_type tuple_{};
    };

    template<class EventBase, class... Args>
    class basic_widget_event : public widget_event_base<EventBase, Args...>
    {
    public:
        using widget_event_base<EventBase, Args...>::widget_event_base;
    };

    template<class EventBase, class... Args>
    class basic_widget_event<EventBase, windowrefwrap_t, Args...> : public widget_event_base<EventBase, windowrefwrap_t, Args...>
    {
    public:
         using widget_event_base<EventBase, windowrefwrap_t, Args...>::widget_event_base;

        constexpr const window& window() const noexcept
        {
            return std::get<windowrefwrap_t>(this->tuple());
        }

        constexpr const_module_handle_t app() const noexcept
        {
            return window().module();
        }

        constexpr window_content content() const noexcept
        {
            return window().content();
        }

        constexpr pxsize2d viewport() const noexcept
        {
            return window().viewport();
        }
    };
}