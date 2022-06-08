#pragma once

#include <core/warnings.h>
#include <core/type_traits.h>


D_WARNING_PUSH;
D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);

template <class E> [[nodiscard]]
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

template<class Out, class In> [[nodiscard]]
constexpr Out underlying_cast(const In& in) noexcept
{
    static_assert(std::is_same_v<remove_cve_t<Out>, remove_cve_t<In>>);
    return static_cast<Out>(in);
}

D_WARNING_POP;

