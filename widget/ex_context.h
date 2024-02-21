#pragma once

#include <widget/fwd.h>


namespace widget
{
    template<class... Types>
    struct ex_context
    {
        template<class Fn, class... Items>
        constexpr decltype(auto) operator () (Fn&& fn, Items&&... items) const noexcept
        {
            if constexpr (std::is_base_of_v<ex_context_type_enumerator, std::remove_reference_t<Fn>>)
            {
                return std::forward<Fn>(fn)(*this, std::forward<Items>(items)...);
            }
            else if constexpr (sizeof...(items))
            {
                return std::forward<Fn>(fn)(std::forward<Items>(items)...);
            }
            else
            {
                D_UNUSED(fn);
                return noapply;
            }
        }

        constexpr noapply_t apply(no_overload) const noexcept
        {
            return noapply;
        }
    };

    template<class... Types>
    constexpr ex_context<Types...> ex_context_v{};
}
