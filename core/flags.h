#pragma once

#include <core/underlying_cast.h>

template<class enum_type>
struct flags
{
    enum_type value;

    explicit constexpr operator bool() const noexcept
    {
        return !!to_underlying(value);
    }
};

template<class enum_type>
constexpr flags<enum_type> operator & ( flags<enum_type> left, enum_type right ) noexcept
{
    return { underlying_cast<enum_type>( to_underlying( left.value ) & to_underlying( right ) ) };
}

template<class enum_type>
constexpr flags<enum_type> operator & ( enum_type left, flags<enum_type> right ) noexcept
{
    return { underlying_cast<enum_type>( to_underlying( left ) & to_underlying( right.value ) ) };
}

template<class enum_type>
constexpr flags<enum_type> operator | ( flags<enum_type> left, enum_type right ) noexcept
{
    return { underlying_cast<enum_type>( to_underlying( left.value ) | to_underlying( right ) ) };
}

template<class enum_type>
constexpr flags<enum_type> operator | ( enum_type left, flags<enum_type> right ) noexcept
{
    return right | left;
}
