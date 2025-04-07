#include <widget/context.h>
#include <widget/ex_context.h>

using widget::redraw_event;
using widget::ex_context_v;


namespace
{
    template<size_t id>
    struct shader
    {
        constexpr shader() noexcept = default;
        D_DISABLE_COPY_CA(shader);
    };

    struct widget0
    {
        using ex_type = redraw_event<
            shader<0>,
            shader<1>,
            buffer_view
        >;

        template<class Fn>
        constexpr decltype(auto) apply(Fn fn) const noexcept
        {
            return ex_context_v<ex_type>(fn);
        }
    };

    struct widget1
    {
        using ex_type = redraw_event<
            shader<1>,
            shader<2>,
            widget::temp_buffer
        >;

        struct subwidget
        {
            constexpr noapply_t apply(no_overload) const noexcept
            {
                return noapply;
            }
        };

        subwidget w;

        template<class Fn>
        constexpr decltype(auto) apply(Fn fn) const noexcept
        {
            return ex_context_v<ex_type>(fn, w);
        }
    };

    struct main_widget
    {
        widget0 w0_0;
        widget0 w0_1;
        widget1 w1;

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(w0_0, w0_1, w1);
        }
    };

    template<class W>
    using context_sample_t = repack_types_t<
        transform_types_t<widget::context_unview_t, widget::decl_contexts_t<typename W::ex_type> >, 
        widget::context
    >;
}


void test_context() noexcept
{
    static_assert(sizeof(widget::basic_initialization_event<>) == sizeof(widget::initialization_event_base));
    static_assert(sizeof(widget::basic_mouse_move_event<>) == sizeof(ui::mouse_move_event));

    {
        using widget::private_detail_context::private_detail_context::private_detail_widgets::widgets_t;

        {
            using main_widgets_t = widgets_t<main_widget>;
            static_assert(4u == types_size_v<main_widgets_t>);
            static_assert(types_has_type_v<main_widget, main_widgets_t>);
            static_assert(types_has_type_v<widget0, main_widgets_t>);
            static_assert(types_has_type_v<widget1, main_widgets_t>);
            static_assert(types_has_type_v<widget1::subwidget, main_widgets_t>);
            static_assert(types_template_is_v<main_widgets_t, types_pack>);
        }

        {
            using context0_t = widget::context_t<widget0>;
            static_assert(4u == types_size_v<context0_t>);
            
            static_assert(types_has_type_v<widget::windowrefwrap_t, context0_t>);
            static_assert(types_has_type_v<shader<0>, context0_t>);
            static_assert(types_has_type_v<shader<1>, context0_t>);
            static_assert(types_has_type_v<widget::temp_buffer, context0_t>);
            static_assert(types_template_is_v<context0_t, widget::context>);

            static_assert(std::is_same_v<context0_t, context_sample_t<widget0> > );
        }

        {
            using context1_t = widget::context_t<widget1>;
            static_assert(4u == types_size_v<context1_t>);
            static_assert(types_has_type_v<widget::windowrefwrap_t, context1_t>);
            static_assert(types_has_type_v<shader<1>, context1_t>);
            static_assert(types_has_type_v<shader<2>, context1_t>);
            static_assert(types_has_type_v<widget::temp_buffer, context1_t>);
            static_assert(types_template_is_v<context1_t, widget::context>);

            static_assert(std::is_same_v<context1_t, context_sample_t<widget1> > );
        }


        {
            using context_t = widget::context_t<main_widget>;
            static_assert(5u == types_size_v<context_t>);
            static_assert(types_has_type_v<widget::windowrefwrap_t, context_t>);
            static_assert(types_has_type_v<shader<0>, context_t>);
            static_assert(types_has_type_v<shader<1>, context_t>);
            static_assert(types_has_type_v<shader<2>, context_t>);
            static_assert(types_has_type_v<widget::temp_buffer, context_t>);
            static_assert(types_template_is_v<context_t, widget::context>);

            widget::window w{};
            context_t cc{ w };
            D_ASSERT(std::addressof(cc.cref_window()) == std::addressof(w));
            D_ASSERT(std::addressof(cc.ref_window()) == std::addressof(w));
        }
    }

    D_ASSERT(!errno);
}