#pragma once

#include <core/tuple_algorithm.h>

#include <widget/draw_context_elements.h>
#include <widget/event.h>


namespace widget
{
    template<class Widget>
    using decl_redraw_event_type_t = typename Widget::redraw_event_type;

    template<class Widget>
    using redraw_event_type_t = detected_or_t<redraw_event<>, decl_redraw_event_type_t, std::remove_cvref_t<Widget> >;

    template<class Widget>
    using redraw_event_tuple_type_t = typename redraw_event_type_t<Widget>::tuple_type;

    template<class Widget>
    using draw_context_tuple_t = transform_tuple_t<unview_t, redraw_event_tuple_type_t<Widget>>;


    template<class... Types>
    class common_draw_context
    {
    public:
        using tuple_type = D_CONDITIONAL_OS_WINDOWS(tuple_sizeof_optimization_t<std::tuple<Types...>>, std::tuple<Types...>);

        template<class T>
        constexpr unview_cref_t<T> cget() const noexcept
        {
            return std::get<unview_t<T>>(tuple_);
        }

        template<class T>
        constexpr unview_cref_t<T> get() const noexcept
        {
            return cget<T>();
        }

        template<class T>
        constexpr unview_ref_t<T> get() noexcept
        {
            return as_mutable(cget<T>());
        }

        template<class... ETypes>
        constexpr operator redraw_event<ETypes...>() const noexcept
        {
            return redraw_event<ETypes...>{ cget<ETypes>()... };
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return std::apply(fn, tuple_);
        }

    private:
        tuple_type tuple_{};
    };


    template<class Tuple>
    struct make_common_draw_context_type;

    template<class... Types>
    struct make_common_draw_context_type<std::tuple<Types...>>
    {
        using type = common_draw_context<unview_t<Types>...>;
    };

    template<class Tuple>
    using make_common_draw_context_t = typename make_common_draw_context_type<Tuple>::type;


    struct forward_ref_types_function
    {
        template<class... Args>
        constexpr std::tuple<Args&...> operator () (Args&... types) const noexcept
        {
            return { types... };
        }
    };

    constexpr forward_ref_types_function forward_ref_types_function_v{};

    template<class W>
    using siblings_widget_ref_tuple_t = decltype(std::declval<W>().apply(forward_ref_types_function_v));

    namespace private_detail_widget_tuple
    {
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
            using type = tuple_push_front_t<P, tuple_cat_t<typename siblings_loop_type<S, siblings_widget_ref_tuple_t<S>>::type...>>;
        };
    }

    template<class W>
    struct widget_ref_tuple_type
    {
        using type = typename private_detail_widget_tuple::siblings_loop_type<W, siblings_widget_ref_tuple_t<W>>::type;
    };

    template<class Widget>
    using widget_ref_tuple_t = typename widget_ref_tuple_type<Widget>::type;


    template<class TupleWidgets>
    struct tuple_common_draw_context_type;

    template<>
    struct tuple_common_draw_context_type<std::tuple<>>
    {
        using type = std::tuple<>;
    };

    template<class T0, class... Types>
    struct tuple_common_draw_context_type<std::tuple<T0, Types...>>
    {
        using type = tuple_unique_push_back_tuple_t<
            draw_context_tuple_t<T0>,
            typename tuple_common_draw_context_type<std::tuple<Types...>>::type
        >;
    };

    template<class TupleWidgets>
    using tuple_common_draw_context_t = typename tuple_common_draw_context_type<TupleWidgets>::type;


    template<class Widget>
    using widget_common_draw_context_t = make_common_draw_context_t<tuple_common_draw_context_t<widget_ref_tuple_t<Widget>>>;
}
