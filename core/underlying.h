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

template<class Target, class Source> [[nodiscard]]
constexpr Target underlying_cast(Source source) noexcept
{
    static_assert(std::disjunction_v<std::is_enum<Target>, std::is_enum<Source>>);
    static_assert(std::is_same_v<remove_cve_t<Target>, remove_cveref_t<Source>>);
    return static_cast<Target>(source);
}

template<class E> [[nodiscard]]
constexpr E e_bit_or(E left, E right) noexcept
{
    using underlying_t = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying_t>(left) | static_cast<underlying_t>(right));
}

template<class E> [[nodiscard]]
constexpr E e_bit_and(E left, E right) noexcept
{
    using underlying_t = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying_t>(left) & static_cast<underlying_t>(right));
}

template<class E> [[nodiscard]]
constexpr E e_bit_not(E e) noexcept
{
    using underlying_t = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<underlying_t>(e));
}

template<class E>
constexpr E& e_bit_and_eq(E& left, E right) noexcept
{
    left = e_bit_and(left, right);
    return left;
}

template<class E> [[nodiscard]]
constexpr bool e_extract(E& e, const E bits) noexcept
{
    return (bits == e_bit_and(e, bits)) && (e_bit_and_eq(e, e_bit_not(bits)), true);
}

D_WARNING_POP;

