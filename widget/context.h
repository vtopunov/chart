#pragma once

#include <core/tuple_algorithm.h>
#include <core/functional.h>

#include <widget/temp_buffer.h>
#include <widget/user_gesture.h>
#include <widget/event.h>
#include <widget/window.h>


namespace widget
{
    namespace private_detail_context_tuple
    {
        template<class Target>
        struct context_source_element_type
        {
            using type = Target;
        };

        template<>
        struct context_source_element_type<buffer_view>
        {
            using type = pix8_temp_buffer;
        };

        template<>
        struct context_source_element_type<window>
        {
            using type = optional_reference_wrapper<window>;
        };

        template<>
        struct context_source_element_type<os::const_module_handle_t>
            : context_source_element_type<window>
        {};

        template<>
        struct context_source_element_type<viewport_size2d>
            : context_source_element_type<window>
        {};

        template<>
        struct context_source_element_type<content_size2d>
            : context_source_element_type<window>
        {};

        template<>
        struct context_source_element_type<ui::user_gesture>
        {
            using type = widget::user_gesture;
        };

        template<class T>
        using context_source_element_t = typename context_source_element_type<T>::type;

        using windowrefwrap_t = context_source_element_t<window>;

        template<class T>
        using decl_context_tuple_t = typename T::context_tuple_type;

        template<class T>
        using context_tuple0_t = detected_or_t<std::tuple<T>, decl_context_tuple_t, T>;

        template<class T>
        using context_tuple_t = transform_tuple_t<context_source_element_t, context_tuple0_t<T> >;

        template<class Ex>
        struct ex_context_tuple_type
        {
            using type = std::tuple<>;
        };

        template<class Ex>
        struct ex_context_tuple_type<ex_context<Ex>>
        {
            using type = context_tuple_t<Ex>;
        };

        template<class Ex>
        using ex_context_tuple_t = typename ex_context_tuple_type<Ex>::type;
    }

    using private_detail_context_tuple::context_source_element_t;
    using private_detail_context_tuple::ex_context_tuple_t;
    using private_detail_context_tuple::windowrefwrap_t;


    template<class... ETypes, class EventBase, class... Types>
    constexpr widget_event<EventBase, ETypes...> make_widget_event(const EventBase& base, const common_context<Types...>& cc) noexcept;

    template<class... Types>
    class common_context
    {
    public:
        using tuple_type = D_CONDITIONAL_OS_WINDOWS(tuple_sizeof_optimization_t<std::tuple<Types...>>, std::tuple<Types...>);

        constexpr explicit common_context(widget::window& window) noexcept
            : tuple_{}
        {
            std::get<windowrefwrap_t>(tuple_) = windowrefwrap_t{ window };
        }

        D_DISABLE_COPYMOVE_CA(common_context);

        template<class T>
        constexpr const T& cget() const noexcept
        {
            return std::get<T>(tuple_);
        }

        template<class T>
        constexpr const T& get() const noexcept
        {
            return cget<T>();
        }

        template<class T>
        constexpr T& get() noexcept
        {
            return as_mutable(cget<T>());
        }

        template<class... ETypes>
        constexpr operator redraw_event<ETypes...>() const noexcept
        {
            return make_widget_event<ETypes...>(redraw_event_base_v, *this);
        }

        constexpr const widget::window& cref_window() const noexcept
        {
            return std::get<windowrefwrap_t>(tuple_);
        }

        constexpr const widget::window& ref_window() const noexcept
        {
            return cref_window();
        }

        constexpr widget::window& ref_window() noexcept
        {
            return as_mutable(cref_window());
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return apply_impl(fn, tuple_, make_tuple_index_sequence<tuple_type>{});
        }

    private:
        template <class Fn, class Tuple, size_t... Indices>
        static constexpr decltype(auto) apply_impl(Fn fn, Tuple& tuple, std::index_sequence<Indices...>) noexcept
        {
            return fn(unrefwrap(std::get<Indices>(tuple))...);
        }

    private:
        tuple_type tuple_;
    };

    template<class... ETypes, class EventBase, class... Types>
    constexpr widget_event<EventBase, ETypes...> make_widget_event(const EventBase& base, const common_context<Types...>& cc) noexcept
    {
        return widget_event<EventBase, ETypes...>{ base, unrefwrap(cc.template cget<context_source_element_t<ETypes>>())... };
    }

    template<class Event, class CommonContext>
    struct event_common_context
    {
        const Event& ui_event;
        const CommonContext& widget_common_context;

        constexpr operator const Event& () const noexcept
        {
            return ui_event;
        }

        template<class... ETypes>
        constexpr operator widget_event<Event, ETypes...>() const noexcept
        {
            return make_widget_event<ETypes...>(ui_event, widget_common_context);
        }
    };

    template<class E, class CC>
    event_common_context(const E&, const CC&) -> event_common_context<E, CC>;


    template<class Tuple>
    struct make_common_context_type;

    template<class... Types>
    struct make_common_context_type<std::tuple<Types...>>
    {
        using type = common_context<Types...>;
    };

    template<class Tuple>
    using make_common_context_t = typename make_common_context_type<Tuple>::type;


    namespace private_detail_widget_tuple
    {
        struct forward_ref_types_function
        {
            template<class... Args>
            constexpr std::tuple<Args&&...> operator () (Args&&... types) const noexcept
            {
                return { std::forward<Args>(types)... };
            }
        };

        constexpr forward_ref_types_function forward_ref_types_function_v{};

        template<class W>
        using siblings_widget_ref_tuple_t = decltype(std::declval<W>().apply(forward_ref_types_function_v));

        template<class W>
        using siblings_widget_tuple_t = transform_tuple_t<std::remove_cvref_t, siblings_widget_ref_tuple_t<W>>;


        template<class P, class S>
        struct siblings_loop_type;

        template<class P>
        struct siblings_loop_type<P, std::tuple<>>
        {
            using type = std::tuple<P>;
        };

        template<class P, class... S>
        struct siblings_loop_type<P, std::tuple<S...>>
        {
            using type = tuple_push_front_t<P, tuple_cat_t<typename siblings_loop_type<S, siblings_widget_tuple_t<S>>::type...>>;
        };


        template<class Widget>
        struct widget_tuple_type
        {
            using widget_type = std::remove_cvref_t<Widget>;
            using type = typename siblings_loop_type<
                widget_type,
                siblings_widget_tuple_t<widget_type>
            >::type;
        };

        template<class Widget>
        using widget_tuple_t = typename widget_tuple_type<Widget>::type;
    }

    using private_detail_widget_tuple::widget_tuple_t;


    template<class TupleWidgets>
    struct tuple_common_context_type;

    template<>
    struct tuple_common_context_type<std::tuple<>>
    {
        using type = std::tuple<windowrefwrap_t>;
    };

    template<class T0, class... Types>
    struct tuple_common_context_type<std::tuple<T0, Types...>>
    {
        using type = tuple_unique_push_back_tuple_t<
            ex_context_tuple_t<T0>,
            typename tuple_common_context_type<std::tuple<Types...>>::type
        >;
    };

    template<class TupleWidgets>
    using tuple_common_context_t = typename tuple_common_context_type<TupleWidgets>::type;


    template<class Widget>
    using common_context_t = make_common_context_t<tuple_common_context_t<unique_tuple_t<widget_tuple_t<Widget>>>>;
}
