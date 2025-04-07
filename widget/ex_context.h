#pragma once

#include <widget/fwd.h>


namespace widget
{
    template<class... Types>
    struct ex_context
    {
        template<class Fn, class... Items>
        constexpr decltype(auto) operator () ([[maybe_unused]] Fn&& fn, [[maybe_unused]] Items&&... items) const noexcept
        {
            if constexpr (std::is_same_v<ex_context_enumerator, std::remove_cvref_t<Fn>>)
            {
                return types_pack_v<Types...>;
            }
            else if constexpr (sizeof...(items))
            {
                return std::forward<Fn>(fn)(std::forward<Items>(items)...);
            }
            else
            {
                return noapply;
            }
        }
    };

    template<class... Types>
    constexpr ex_context<Types...> ex_context_v{};
}
