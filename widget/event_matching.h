#pragma once

#include <widget/fwd.h>


namespace widget
{
    template<class ER = void>
    struct event_result_processor
    {
        static_assert(!std::is_const_v<ER>);
        static_assert(!std::is_reference_v<ER>);

        constexpr event_result_processor(ER right) noexcept
            : result{ right }
        {}

        template<class T, class E>
        event_result_processor(T& function, const E& e) noexcept
            : result{ function(e) }
        {}

        ER result;
    };

    template<>
    struct event_result_processor<void>
    {
        template<class T, class E>
        event_result_processor(T& function, const E& e) noexcept
        {
            function(e);
        }

        constexpr event_result_processor() noexcept = default;
    };

    template<class T>
    event_result_processor(T) -> event_result_processor<std::remove_const_t<T>>;

    constexpr event_result_processor<> no_event_result_processor{};

    [[nodiscard]]
    constexpr event_result_processor<> operator | (event_result_processor<>, event_result_processor<>) noexcept
    {
        return no_event_result_processor;
    }

    template<class ERR>
    [[nodiscard]] constexpr event_result_processor<ERR> operator | (event_result_processor<ERR> left, event_result_processor<>) noexcept
    {
        return left;
    }

    template<class ERL>
    [[nodiscard]] constexpr event_result_processor<ERL> operator | (event_result_processor<>, event_result_processor<ERL> right) noexcept
    {
        return right;
    }

    template<class ERR, class ERL>
    [[nodiscard]] constexpr decltype(auto) operator | (event_result_processor<ERR> left, event_result_processor<ERL> right) noexcept
    {
        return event_result_processor{ left.result | right.result };
    }

    [[nodiscard]]
    constexpr event_result_processor<bool> operator | (event_result_processor<bool> left, event_result_processor<bool> right) noexcept
    {
        return { left.result && right.result };
    }

    template<class Cache, class ER>
    constexpr void write_event_result(Cache& cache, event_result_processor<ER> result) noexcept
    {
        cache = result.result;
    }

    template<class Cache>
    constexpr void write_event_result(Cache& cache, event_result_processor<>) noexcept
    {}

    template<class Cache, class ER>
    constexpr void combine_event_result(Cache& cache, event_result_processor<ER> result) noexcept
    {
        const auto combined_result = event_result_processor<Cache>{ cache } | result;
        write_event_result(cache, combined_result);
    }

    template<class T, class E>
    [[nodiscard]] auto call_widget_event(T& function, const E& e) -> event_result_processor<std::remove_const_t<decltype(function(e))>>
    {
        return { function, e };
    }

    [[nodiscard]] event_result_processor<> call_widget_event(no_overloaded, no_overloaded)
    {
        return {};
    }

    template<class Widget, class Event>
    [[nodiscard]] decltype(auto) apply_event(Widget& wgt, const Event& e) noexcept
    {
        return call_widget_event(wgt, e) 
             | wgt.apply([&e] (auto&... wgts) noexcept { return (no_event_result_processor | ... | apply_event(wgts, e)); });
    }
}
