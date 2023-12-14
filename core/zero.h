#pragma once

#include <type_traits>


namespace private_detail_zero
{
    template<class T, class = void>
    struct zero_type
    {
        static constexpr T value{};
    };

    template<class T>
    struct zero_type<T, std::void_t<decltype(T::zero())>>
    {
        static constexpr T value{ T::zero() };
    };

    struct zero_value_type
    {
        template<class T>
        constexpr operator T () const noexcept
        {
            return zero_type<std::remove_cvref_t<T>>::value;
        }

        template<class T>
        constexpr bool operator == (const T& value) const noexcept
        {
            return static_cast<T>(*this) == value;
        }

        template<class T>
        constexpr bool operator != (const T& value) const noexcept
        {
            return static_cast<T>(*this) != value;
        }
    };

    template<class T>
    constexpr bool operator == (const T& left, const zero_value_type right)noexcept
    {
        return right == left;
    }

    template<class T>
    constexpr bool operator != (const T& left, const zero_value_type right) noexcept
    {
        return right != left;
    }

    template<>
    struct zero_type<void, void>
    {
        static constexpr zero_value_type value{};
    };
}

template<class T = void>
constexpr auto zero_v = private_detail_zero::zero_type<std::remove_cvref_t<T>>::value;


template<class T>
[[nodiscard]] constexpr auto is_positive(const T& value) noexcept -> decltype(zero_v<T> < value)
{
    return zero_v<T> < value;
}

template<class T>
[[nodiscard]] constexpr auto is_negative(const T& value) noexcept -> decltype(value < zero_v<T>)
{
    return value < zero_v<T>;
}


template <class T0, class T1>
[[nodiscard]] std::add_rvalue_reference_t<std::common_type_t<T0, T1>> declcommontype(const T0&, const T1&) noexcept;

template<class T>
[[nodiscard]] constexpr auto constexpr_abs(const T& value) noexcept -> std::remove_reference_t<decltype((is_negative(value), declcommontype(value, -value)))>
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