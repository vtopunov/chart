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
            struct context_unview_type<buffer_view>
            {
                using type = temp_buffer;
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
            using context_unview_t = typename context_unview_type<T>::type;
        }
    }

    using private_detail_context::private_detail_context_unview::context_unview_t;


    template<class... Types>
    class context
    {
    public:
        using tuple_type = D_OS_WINDOWS_OR(types_sizeof_optimization_t<std::tuple<Types...>>, std::tuple<Types...>);

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
            return fn(unrefwrap(std::get<Indices>(tuple_))...);
        }

    private:
        tuple_type tuple_;
    };

    template<class... ETypes, class EventBase, class... Types>
    constexpr basic_widget_event<EventBase, ETypes...> make_widget_event(const EventBase& base, const context<Types...>& cc) noexcept
    {
        return basic_widget_event<EventBase, ETypes...>{ base, unrefwrap(cc.template cget<context_unview_t<ETypes>>())... };
    }

    template<class Event, class CommonContext>
    struct widget_event_factory
    {
        const Event& ui_event;
        const CommonContext& widget_common_context;

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
                struct siblings_loop_type<P, types_pack<>>
                {
                    using type = add_template_t<types_pack, P>;
                };

                template<class P, class... S>
                struct siblings_loop_type<P, types_pack<S...>>
                {
                    using type = types_cat_t<P, typename siblings_loop_type<S, subtypes_t<S>>::type...>;
                };

                template<class Widget>
                struct widgets
                {
                    using widget_type = std::remove_cvref_t<Widget>;
                    using type = types_unique_t<typename siblings_loop_type<
                        widget_type,
                        subtypes_t<widget_type>
                    >::type>;
                };

                template<class Widget>
                using widgets_t = typename widgets<Widget>::type;
            }

            using private_detail_widgets::widgets_t;

            template<class SubtypesOrContext>
            using contexts_t = detected_or_t<add_template_t<types_pack, SubtypesOrContext>, decl_contexts_t, SubtypesOrContext>;

            template<class Widget>
            using ex_subtypes_t = subapply_result_t<Widget, ex_context_enumerator>;

            template<class Widgets>
            using ex_subtypes_sol_t = types_sol_t<transform_types_t<ex_subtypes_t, Widgets> >;

            template<class Contexts>
            using contexts_sol_t = types_sol_t<transform_types_t<contexts_t, Contexts> >;

            template<class Contexts>
            using contexts_unview_t = transform_types_t<context_unview_t, Contexts>;

            template<class Contexts>
            using contexts_unique_t = types_unique_push_back_pack_t<types_pack, types_pack<windowrefwrap_t>, Contexts>;

            template<class Widgets>
            using ex_contexts_sol_t = contexts_sol_t<ex_subtypes_sol_t<Widgets>>;

            template<class Widgets>
            using ex_contexts_unique_unview_sol_t = contexts_unique_t<contexts_unview_t<ex_contexts_sol_t<Widgets> > >;

            template<class Widget>
            using ex_contexts_t = ex_contexts_unique_unview_sol_t<widgets_t<Widget> >;

            template<class Widget>
            using context_t = repack_types_t<ex_contexts_t<Widget>, context>;
        }
    }

    using private_detail_context::private_detail_context::context_t;
}
