#include <widget/event_matching.h>
#include <widget/window.h>


namespace
{
    struct widget0
    {
        size_t n_calls{ 0 };

        void operator () (ui::const_module_handle_t) noexcept
        {
            ++n_calls;
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }
    };

    struct widget1
    {
        size_t n_calls{ 0 };

        void operator () (viewport_size2d) noexcept
        {
            ++n_calls;
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }
    };

    struct main_widget
    {
        widget0 w0{};
        widget1 w1{};

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn(w0, w1);
        }
    };
}

void test_event_matching() noexcept
{
    widget::window testnullw{};
    main_widget main_wgt{};

    {
        D_ASSERT(0u == main_wgt.w0.n_calls);
        D_ASSERT(0u == main_wgt.w1.n_calls);
        D_UNUSED(widget::apply_event(main_wgt, testnullw));
        D_ASSERT(1u == main_wgt.w0.n_calls);
        D_ASSERT(1u == main_wgt.w1.n_calls);
    }
}