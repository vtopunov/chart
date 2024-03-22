#pragma once

#include <widget/fwd.h>


namespace widget
{
    template<class ER>
    struct event_result_processor
    {
        static_assert(!std::is_const_v<ER>);
        static_assert(!std::is_reference_v<ER>);

        constexpr event_result_processor(ER right) noexcept
            : result{ right }
        {}

        template<class T, class E>
        event_result_processor(T&& function, const E& e) noexcept
            : result{ std::forward<T>(function)(e) }
        {}

        ER result;
    };

    template<>
    struct event_result_processor<void>
    {
        template<class T, class E>
        event_result_processor(T&& function, const E& e) noexcept
        {
            std::forward<T>(function)(e);
        }
    };

    template<>
    struct event_result_processor<noapply_t>
    {
        constexpr event_result_processor(no_overload, no_overload) noexcept
        {}
    };

    using no_event_result_processor_t = event_result_processor<noapply_t>;

    constexpr no_event_result_processor_t no_event_result_processor{ nullptr, nullptr };

    namespace private_detail_has_event_result
    {
        template<class ERP>
        using decl_result_member_t = decltype(std::declval<const ERP&>().result);
    }

    template<class ER>
    using has_event_result = is_detected<private_detail_has_event_result::decl_result_member_t, event_result_processor<ER>>;

    template<class ER>
    constexpr auto has_event_result_v = has_event_result<ER>::value;

    template<class ERL, class ERR>
    [[nodiscard]] constexpr decltype(auto) operator | (event_result_processor<ERL> left, event_result_processor<ERR> right) noexcept
    {
        constexpr auto left_has_result = has_event_result_v<ERL>;
        constexpr auto right_has_result = has_event_result_v<ERR>;

        if constexpr (left_has_result)
        {
            if constexpr (right_has_result)
            {
                if constexpr (std::conjunction_v<is_same_uncvref<bool, ERR>, is_same_uncvref<bool, ERL>>)
                {
                    const bool result{ left.result && right.result };
                    return event_result_processor<bool>{ result };
                }
                else
                {
                    using common_t = std::remove_cvref_t<decltype(left.result | right.result)>;
                    return event_result_processor<common_t>{ left.result | right.result };
                }
            }
            else
            {
                return left;
            }
        }
        else
        {
            if constexpr (right_has_result)
            {
                return right;
            }
            else
            {
                return no_event_result_processor;
            }
        }
    }

    template<class ER>
    [[nodiscard]] constexpr decltype(auto) operator | (event_result_processor<ER> left, noapply_t) noexcept
    {
        return left;
    }

    template<class ER>
    [[nodiscard]] constexpr decltype(auto) operator | (noapply_t, event_result_processor<ER> right) noexcept
    {
        return right;
    }

    template<class Cache, class ER>
    constexpr void write_event_result(Cache& cache, event_result_processor<ER> result) noexcept
    {
        if constexpr (has_event_result_v<ER>)
        {
            cache = result.result;
        }
    }

    template<class Cache, class ER>
    constexpr void combine_event_result(Cache& cache, event_result_processor<ER> result) noexcept
    {
        const auto combined_result = event_result_processor<Cache>{ cache } | result;
        write_event_result(cache, combined_result);
    }

    template<class T, class E>
    [[nodiscard]] auto call_widget_event(T&& function, const E& e) noexcept 
        -> event_result_processor<std::remove_const_t<decltype(std::forward<T>(function)(e))>>
    {
        return { std::forward<T>(function), e };
    }

    [[nodiscard]] constexpr no_event_result_processor_t call_widget_event(no_overload, no_overload) noexcept
    {
        return no_event_result_processor;
    }

    template<class Widget, class Event>
    [[nodiscard]] decltype(auto) apply_event(Widget&& wgt, const Event& e) noexcept
    {
        return call_widget_event(std::forward<Widget>(wgt), e) | std::forward<Widget>(wgt).apply([&e] <class... W> (W&&... wgts) noexcept
        {
            return (no_event_result_processor | ... | apply_event(std::forward<W>(wgts), e));
        });
    }
}
