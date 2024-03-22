#include <widget/event_matching.h>
#include <widget/context.h>


namespace
{
    template<class Event>
    struct e_widget
    {
        size_t n_calls{ 0 };

        void operator () (Event) noexcept
        {
            ++n_calls;
        }

        constexpr widget::noapply_t apply(no_overload) const noexcept
        {
            return widget::noapply;
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
            return fn(w0, w1, w2);
        }
    };
}

void test_event_matching() noexcept
{
    const widget::window w{};
    const widget::common_context_t<main_widget> cc{ w };
    const widget::event_common_context e{ widget::viewport_event_base_v, cc};
    main_widget main_wgt{};

    {
        D_ASSERT(0u == main_wgt.w0.n_calls);
        D_ASSERT(0u == main_wgt.w1.n_calls);
        D_ASSERT(0u == main_wgt.w2.n_calls);
        D_UNUSED(widget::apply_event(main_wgt, e));
        D_ASSERT(1u == main_wgt.w0.n_calls);
        D_ASSERT(1u == main_wgt.w1.n_calls);
        D_ASSERT(1u == main_wgt.w2.n_calls);
    }
}