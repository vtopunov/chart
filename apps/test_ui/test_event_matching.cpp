#include <ui/event_matching.h>

#include <os/os.h>

namespace
{
    struct no_processor
    {};

    struct size_event_processor
    {
        ui::event_result_opt_t result{};
        std::optional<ui::size_event> last_se{};
        size_t count_call{ 0 };

        ui::event_result_opt_t operator () (const ui::size_event& e) noexcept
        {
            last_se = e;
            ++count_call;
            return result;
        }
    };

    struct const_event_processor
    {
        mutable size_t const_count_call{ 0 };
        size_t mutable_count_call{ 0 };

        void operator () (const ui::size_event&) const noexcept
        {
            ++const_count_call;
        }

        void operator () (const ui::event&) noexcept
        {
            ++mutable_count_call;
        }
    };

    template<class Result>
    struct event_result_processor
    {
        using result_or_dummy_type = std::conditional_t<std::is_void_v<Result>, dummy, Result>;

        size_t count_call{ 0 };
        D_NO_UNIQUE_ADDRESS result_or_dummy_type result{};

        Result operator () (const ui::size_event&) noexcept
        {
            ++count_call;

            if constexpr (!std::is_void_v<Result>)
            {
                return result;
            }
        }
    };

    struct event_size_event_processor
    {
        ui::event_result_opt_t result{};
        std::optional<ui::size_event> last_se{};
        std::optional<ui::event> last_e{};
        size_t count_size_event_call{ 0 };
        size_t count_event_call{ 0 };

        ui::event_result_opt_t operator () (const ui::size_event& e) noexcept
        {
            last_se = e;
            ++count_size_event_call;
            return result;
        }

        ui::event_result_opt_t operator () (const ui::event& e) noexcept
        {
            last_e = e;
            ++count_event_call;
            return result;
        }
    };

    struct base_custom_event
    {
        struct base_sub_event {};

        constexpr operator base_sub_event () const noexcept
        {
            return {};
        }
    };

    struct custom_event : base_custom_event
    {
        struct sub_event {};

        constexpr operator sub_event () const noexcept
        {
            return {};
        }
    };

    struct ce_processor
    {
        ui::event_result_opt_t result{};
        size_t count_call{ 0 };

        ui::event_result_opt_t operator () (custom_event) noexcept
        {
            ++count_call;
            return result;
        }
    };

    struct cesub_processor
    {
        ui::event_result_opt_t result{};
        size_t count_call{ 0 };

        ui::event_result_opt_t operator () (custom_event::sub_event) noexcept
        {
            ++count_call;
            return result;
        }
    };

    struct cebasesub_processor
    {
        ui::event_result_opt_t result{};
        size_t count_call{ 0 };

        ui::event_result_opt_t operator () (custom_event::base_sub_event) noexcept
        {
            ++count_call;
            return result;
        }
    };

    template<class T, class E>
    auto call_c_event(T& function, const E& e) noexcept -> decltype(function(e))
    {
        return function(e);
    }

    template<class T>
    constexpr std::nullopt_t call_c_event(T&, no_overload) noexcept
    {
        return std::nullopt;
    }
}


void test_event_matching() noexcept
{    
    constexpr auto make_size_event = [] (ui::window_handle_t w, pxsizes sizes) noexcept
    {
        return ui::size_event{ w, ui::event_style::size, 0u, MAKELPARAM(sizes.width(), sizes.height()) };
    };

    constexpr auto se12 = make_size_event(nullptr, { 1_npx, 2_npx });
    constexpr auto se23 = make_size_event(nullptr, { 2_npx, 3_npx });

    {
        no_processor noproc{};
        static_assert(std::is_same_v<decltype(ui::invoke_event(noproc, se12)), std::nullopt_t>);
    }

    {
        const_event_processor cproc{};
        const auto r = ui::invoke_event(cproc, se12);
        D_ASSERT(1u == cproc.const_count_call);
        D_ASSERT(0u == cproc.mutable_count_call);
    }

    {
        {
            constexpr ui::event_result_t test_r{ 123 };
            event_result_processor<ui::event_result_t> erproc{ .result{ test_r } };
            const auto r = ui::invoke_event(erproc, se12);
            static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
            D_ASSERT(r.has_value() && (test_r == *r) && (test_r == erproc.result));
            D_ASSERT(1u == erproc.count_call);
        }

        {
            constexpr char test_r{ 123 };
            event_result_processor<char> erproc{ .result{ test_r } };
            const auto r = ui::invoke_event(erproc, se12);
            static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
            D_ASSERT(r.has_value() && (test_r == *r) && (test_r == erproc.result));
            D_ASSERT(1u == erproc.count_call);
        }

        {
            static constexpr ui::event_result_t dummy_mem_r{ 123 };
            constexpr const ui::event_result_t*const test_r = &dummy_mem_r;
            event_result_processor<ui::event_result_t*> erproc{ .result{ as_mutable_pointer(test_r) } };
            const auto r = ui::invoke_event(erproc, se12);
            static_assert(std::is_same_v<decltype(r), const no_convertible_t>);
            D_ASSERT(1u == erproc.count_call);
        }

        {
            event_result_processor<std::nullopt_t> erproc{ .result{ std::nullopt } };
            const auto r = ui::invoke_event(erproc, se12);
            static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
            D_ASSERT(!r.has_value() && r == erproc.result && std::nullopt == r);
            D_ASSERT(1u == erproc.count_call);
        }

        {
            constexpr ui::event_result_t test_r{ 123 };
            event_result_processor<ui::event_result_opt_t> erproc{ .result{ test_r } };
            const auto r = ui::invoke_event(erproc, se12);
            static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
            D_ASSERT(r.has_value() && (test_r == *r) && (test_r == erproc.result));
            D_ASSERT(1u == erproc.count_call);
        }

        {
            constexpr ui::event_result_t test_r{ 123 };
            event_result_processor<ui::event_result_opt_t> erproc{ .result{ test_r } };
            const auto r = ui::invoke_event(erproc, static_cast<const ui::event&>(se12));
            static_assert(std::is_same_v<decltype(r), const std::nullopt_t>);
            D_ASSERT(erproc.result.has_value() && (r != erproc.result) && (test_r == erproc.result));
            D_ASSERT(0u == erproc.count_call);
        }
    }

    {
        size_event_processor sproc{};
        const auto r = ui::invoke_event(sproc, se12);
        static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
        D_ASSERT(std::nullopt == r);
        D_ASSERT(1u == sproc.count_call);
        D_ASSERT(sproc.last_se.has_value() && (se12.sizes() == sproc.last_se->sizes()));
    }

    {
        constexpr ui::event_result_t test_r{ 123 };
        size_event_processor sproc{ .result{ test_r } };
        const auto r = ui::invoke_event(sproc, se23);
        static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
        D_ASSERT(r == sproc.result && r.has_value() && test_r == *r);
        D_ASSERT(1u == sproc.count_call);
        D_ASSERT(sproc.last_se.has_value() && (se23.sizes() == sproc.last_se->sizes()));
    }


    {
        constexpr ui::event_result_t test_r{ 234 };
        event_size_event_processor esproc{ .result{ test_r } };
        const auto r = ui::invoke_event(esproc, se23);
        static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
        D_ASSERT(r == esproc.result && r.has_value() && test_r == *r);
        D_ASSERT(0u == esproc.count_event_call);
        D_ASSERT(1u == esproc.count_size_event_call);
        D_ASSERT(esproc.last_se.has_value() && (se23.sizes() == esproc.last_se->sizes()));
    }

    {
        constexpr ui::event_result_t test_r{ 123 };
        custom_event ce{};
        ce_processor ceproc{ .result{ test_r } };
        const auto r = call_c_event(ceproc, ce);
        static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
        D_ASSERT(r == ceproc.result && r.has_value() && test_r == *r);
        D_ASSERT(1u == ceproc.count_call);
    }

    {
        constexpr ui::event_result_t test_r{ 123 };
        custom_event::sub_event ces{};
        cesub_processor cesubproc{ .result{ test_r } };
        const auto r = call_c_event(cesubproc, ces);
        static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
        D_ASSERT(r == cesubproc.result && r.has_value() && test_r == *r);
        D_ASSERT(1u == cesubproc.count_call);
    }

    {
        constexpr ui::event_result_t test_r{ 123 };
        custom_event::base_sub_event cebs{};
        cebasesub_processor cebsubproc{ .result{ test_r } };
        const auto r = call_c_event(cebsubproc, cebs);
        static_assert(std::is_same_v<decltype(r), const ui::event_result_opt_t>);
        D_ASSERT(r == cebsubproc.result && r.has_value() && test_r == *r);
        D_ASSERT(1u == cebsubproc.count_call);
    }
}