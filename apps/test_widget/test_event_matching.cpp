#include <widget/event_matching.h>
#include <widget/context.h>
#include <widget/ex_context.h>


namespace
{
    static size_t viewport_n_calls{ 0 };

    struct dummy_exports
    {
        static constexpr dummy apply(no_overload) noexcept
        {
            return dummy_v;
        }

        template<class>
        struct interface {};
    };

    struct dummy_viewport_exports
    {
        static constexpr dummy apply(no_overload) noexcept
        {
            return dummy_v;
        }

        template<class>
        struct interface
        {
            void viewport(pxsizes) const noexcept
            {
                ++viewport_n_calls;
            }
        };
    };

    struct dummy_vertex_shader
    {
        static constexpr auto source = ""_vert_glsl;
        using exports = dummy_exports;
    };

    struct dummy_viewport_vertex_shader
    {
        static constexpr auto source = ""_vert_glsl;
        using exports = dummy_viewport_exports;
    };

    struct dummy_fragment_shader
    {
        static constexpr auto source = ""_frag_glsl;
        using exports = dummy_exports;
    };

    using dummy_shader_libarary = shader_library<dummy_vertex_shader, dummy_fragment_shader>;
    using dummy_viewport_shader_libarary = shader_library<dummy_viewport_vertex_shader, dummy_fragment_shader>;

    template<class Event>
    struct e_widget
    {
        size_t n_calls{ 0 };

        void operator () (Event) noexcept
        {
            ++n_calls;
        }

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }
    };

    struct main_widget
    {
        e_widget<widget::viewport_event_base > w0{};
        e_widget<widget::basic_viewport_event<> > w1{};
        e_widget<widget::viewport_event<> >  w2{};

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return widget::ex_context_v<
                dummy_shader_libarary,
                dummy_viewport_shader_libarary
            >(fn, w0, w1, w2);
        }
    };
}

void test_event_matching() noexcept
{
    widget::window w{};
    widget::context_t<main_widget> cc{ w };
    const widget::widget_event_factory e{ widget::viewport_event_base_v, cc };
    main_widget main_wgt{};

    {
        D_ASSERT(0u == main_wgt.w0.n_calls);
        D_ASSERT(0u == main_wgt.w1.n_calls);
        D_ASSERT(0u == main_wgt.w2.n_calls);
        D_ASSERT(0u == viewport_n_calls);
        D_UNUSED(widget::apply_event(cc, e));
        D_ASSERT(0u == main_wgt.w0.n_calls);
        D_ASSERT(0u == main_wgt.w1.n_calls);
        D_ASSERT(0u == main_wgt.w2.n_calls);
        D_ASSERT(1u == viewport_n_calls);
        D_UNUSED(widget::apply_event(main_wgt, e));
        D_ASSERT(1u == main_wgt.w0.n_calls);
        D_ASSERT(1u == main_wgt.w1.n_calls);
        D_ASSERT(1u == main_wgt.w2.n_calls);
        D_ASSERT(1u == viewport_n_calls);
    }
}