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

        template<class Fn>
        constexpr decltype(auto) apply(Fn fn) const noexcept
        {
            return ex_context_v<ex_type>(fn);
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

    template<class ExContext, class ExType>
    constexpr bool test_ex_context0_v = std::conjunction_v<
        std::is_same<ExContext, widget::ex_context<ExType> >,
        std::is_same<widget::ex_context_tuple_t<ExContext>, transform_types_t<widget::context_source_element_t, typename ExType::context_tuple_type> >
    >;

    template<class ExContext, class TestWidget>
    constexpr bool test_ex_context_v = test_ex_context0_v<ExContext, typename TestWidget::ex_type>;
}


void test_context() noexcept
{
    static_assert(sizeof(widget::basic_initialization_event<>) == sizeof(widget::initialization_event_base));
    static_assert(sizeof(widget::basic_mouse_move_event<>) == sizeof(ui::mouse_move_event));

    {
        using cc_t = widget::common_context_t<main_widget>;
        using tuple_cc_t = typename cc_t::tuple_type;

        static_assert(types_has_type_v<shader<0>, tuple_cc_t>);
        static_assert(types_has_type_v<shader<1>, tuple_cc_t>);
        static_assert(types_has_type_v<shader<2>, tuple_cc_t>);
        static_assert(types_has_type_v<widget::temp_buffer, tuple_cc_t>);
        static_assert(types_has_type_v<widget::windowrefwrap_t, tuple_cc_t>);
        static_assert(5u == std::tuple_size_v<tuple_cc_t>);

        widget::window w{};
        cc_t cc{ w };
        D_ASSERT(std::addressof(cc.cref_window()) == std::addressof(w));
        D_ASSERT(std::addressof(cc.ref_window()) == std::addressof(w));
    }

    using ex_context0_t = std::tuple_element_t<1u, widget::widget_tuple_t<widget0>>;
    using ex_context1_t = std::tuple_element_t<1u, widget::widget_tuple_t<widget1>>;

    static_assert(test_ex_context_v<ex_context0_t, widget0>);
    static_assert(test_ex_context_v<ex_context1_t, widget1>);

    static_assert(std::is_same_v<unique_tuple_t<widget::widget_tuple_t<main_widget>>, std::tuple<main_widget, widget0, ex_context0_t, widget1, ex_context1_t> >);

    D_ASSERT(!errno);
}