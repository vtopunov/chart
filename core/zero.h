#pragma once

#include <type_traits>

template<class T>
struct zero_instance
{
    static constexpr T value{};
};

template<class T>
constexpr auto zero_v = zero_instance<std::remove_cvref_t<T>>::value;

template<class T>
constexpr decltype(auto) is_positive(const T& value) noexcept
{
    return zero_v<T> < value;
}

template<class T>
constexpr decltype(auto) is_negative(const T& value) noexcept
{
    return value < zero_v<T>;
}

template<class T> [[nodiscard]]
constexpr T constexpr_abs(T value) noexcept
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return value;
    }
    else
    {
        return is_negative(value) ? -value : value;
    }
}
