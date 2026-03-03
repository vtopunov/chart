#pragma once

#include <widget/temp_buffer.h>
#include <widget/gesture.h>
#include <widget/shader.h>


namespace widget
{
    namespace private_detail_context
    {
        namespace private_detail_context_unview
        {
            template<class Target>
            struct context_unview_type
            {
                using type = Target;
            };

            template<>
            struct context_unview_type<widget::window>
            {
                using type = windowrefwrap_t;
            };

            template<>
            struct context_unview_type<ui::gesture>
            {
                using type = widget::gesture;
            };

            template<class VS, class FS>
            struct context_unview_type<shader_library<VS, FS> >
            {
                using type = widget::widget_shader_library<VS, FS>;
            };

            template<class T>
            using context_unview_t = typename context_unview_type<std::remove_const_t<T>>::type;
        }
    }

    using private_detail_context::private_detail_context_unview::context_unview_t;


    template<class... Types>
    class context
    {
    public:
        using tuple_type = D_OS_WINDOWS_OR(ttypes_sizeof_optimization_t<std::tuple<Types...>>, std::tuple<Types...>);

        constexpr explicit context(const widget::window& window) noexcept
            : tuple_{}
        {
            std::get<windowrefwrap_t>(tuple_) = windowrefwrap_t{ window };
        }

        D_DISABLE_COPYMOVE_CA(context);

        template<class T>
        [[nodiscard]] constexpr const T& cget() const noexcept
        {
            return std::get<T>(tuple_);
        }

        template<class T>
        [[nodiscard]] constexpr const T& get() const noexcept
        {
            return cget<T>();
        }

        template<class T>
        [[nodiscard]] constexpr T& get() noexcept
        {
            return as_mutable(cget<T>());
        }

        [[nodiscard]] constexpr const widget::window& cref_window() const noexcept
        {
            return std::get<windowrefwrap_t>(tuple_);
        }

        [[nodiscard]] constexpr const widget::window& ref_window() const noexcept
        {
            return cref_window();
        }

        [[nodiscard]] constexpr widget::window& ref_window() noexcept
        {
            return as_mutable(cref_window());
        }

        template<class Fn>
        constexpr decltype(auto) apply(Fn&& fn) noexcept
        {
            return apply_impl(fn, std::index_sequence_for<Types...>{});
        }

    private:
        template <class Fn, size_t... Indices>
        constexpr decltype(auto) apply_impl(Fn& fn, std::index_sequence<Indices...>) noexcept
        {
            return fn(::unrefwrap(std::get<Indices>(tuple_))...);
        }

    private:
        tuple_type tuple_;
    };

    template<class... ETypes, class EventBase, class... Types>
    [[nodiscard]] constexpr basic_widget_event<EventBase, ETypes...> make_widget_event(const EventBase& base, context<Types...>& cc) noexcept
    {
        return basic_widget_event<EventBase, ETypes...>{ base, ::unrefwrap(cc.template get<context_unview_t<ETypes>>())... };
    }

    template<class Event, class CommonContext>
    struct widget_event_factory
    {
        const Event& ui_event;
        CommonContext& widget_common_context;

        constexpr operator const Event& () const noexcept
        {
            return ui_event;
        }

        template<class... ETypes>
        constexpr operator basic_widget_event<Event, ETypes...>() const noexcept
        {
            return make_widget_event<ETypes...>(ui_event, widget_common_context);
        }
    };

    template<class E, class CC>
    widget_event_factory(const E&, const CC&) -> widget_event_factory<E, CC>;


    namespace private_detail_context
    {
        namespace private_detail_context
        {
            namespace private_detail_widgets
            {
                template<class P, class S>
                struct siblings_loop_type;

                template<class P>
                struct siblings_loop_type<P, ttypes<>>
                {
                    using type = add_template_t<ttypes, P>;
                };

                template<class P, class... S>
                struct siblings_loop_type<P, ttypes<S...>>
                {
                    using type = types_cat_t<P, typename siblings_loop_type<S, subtypes_t<S>>::type...>;
                };

                template<class Widget>
                struct widgets
                {
                    using widget_type = std::remove_cvref_t<Widget>;
                    using type = ttypes_unique_t<typename siblings_loop_type<
                        widget_type,
                        subtypes_t<widget_type>
                    >::type>;
                };

                template<class Widget>
                using widgets_t = typename widgets<Widget>::type;
            }

            using private_detail_widgets::widgets_t;

            template<class SubtypesOrContext>
            using contexts_t = detected_or_t<add_template_t<ttypes, SubtypesOrContext>, decl_context_t, SubtypesOrContext>;

            template<class Widget>
            using ex_subtypes_t = subapply_result_t<Widget, ex_context_enumerator>;

            template<class Widgets>
            using ex_subtypes_sol_t = types_sol_t<ttypes_transform_t<ex_subtypes_t, Widgets> >;

            template<class Contexts>
            using contexts_sol_t = types_sol_t<ttypes_transform_t<contexts_t, Contexts> >;

            template<class Contexts>
            using contexts_unview_t = ttypes_transform_t<context_unview_t, Contexts>;

            template<class Contexts>
            using contexts_unique_t = ttypes_unique_push_back_ttypes_t<ttypes, ttypes<windowrefwrap_t>, Contexts>;

            template<class Widgets>
            using ex_contexts_sol_t = contexts_sol_t<ex_subtypes_sol_t<Widgets>>;

            template<class Widgets>
            using ex_contexts_unique_unview_sol_t = contexts_unique_t<contexts_unview_t<ex_contexts_sol_t<Widgets> > >;

            template<class Widget>
            using ex_contexts_t = ex_contexts_unique_unview_sol_t<widgets_t<Widget> >;

            template<class Widget>
            using context_t = ttypes_repack_t<ex_contexts_t<Widget>, context>;
        }
    }

    using private_detail_context::private_detail_context::context_t;
}
