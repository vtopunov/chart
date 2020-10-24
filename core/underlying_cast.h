#pragma once

#include <core/narrow_cast.h>

template <class E> [[nodiscard]]
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>( e );
}

template <bool, class T>
struct underlying_type_if
{
    using type = T;
};

template <class T>
struct underlying_type_if<true, T>
{
    using type = std::underlying_type_t<T>;
};

template <bool condition, class T>
using underlying_type_if_t = typename underlying_type_if<condition, T>::type;

template <class T>
struct remove_enum
{
    using type = underlying_type_if_t<std::is_enum_v<T>, T>;
};

template <class T>
using remove_enum_t = typename remove_enum<T>::type;

template<class Target, class Source>
constexpr bool is_safe_underlying_conversion_v = ( std::is_enum_v<Target> || std::is_enum_v<Source> )
&& is_safe_integral_conversion_v<remove_enum_t<Target>, remove_enum_t<Source>>;

template<class Target, class Source> [[nodiscard]]
constexpr std::enable_if_t<is_safe_underlying_conversion_v<Target, Source>, Target> underlying_cast(Source value) noexcept
{
    return static_cast<Target>( value );
}