#pragma once

#include <core/narrow_cast.h>

namespace font
{
    template<class T, size_t FractBits>
    struct fixed
    {
        using value_type = T;
        static constexpr size_t fract_bits{ FractBits };

        value_type value;

        template<class Int>
        [[nodiscard]] static constexpr std::enable_if_t<std::is_integral_v<Int>, fixed> instance(Int int_value) noexcept
        {
            return { safe_numeric_cast<value_type>(int_value) << fract_bits };
        }

        template<class NewT>
        [[nodiscard]] static constexpr fixed instance(fixed<NewT, FractBits> fix_value) noexcept
        {
            return { narrow_cast<NewT>(fix_value.value) };
        }

        template<class Out>
        [[nodiscard]] constexpr Out narrow_to() const noexcept
        {
            static_assert(std::is_integral_v<Out>);
            return narrow_cast<Out>(discard_fraction());
        }

        constexpr value_type discard_fraction() const noexcept
        {
            return value >> fract_bits;
        }

        constexpr fixed& operator += (const fixed& right) noexcept
        {
            value += right.value;
            return *this;
        }

        constexpr explicit operator bool() const noexcept
        {
            return !!value;
        }

        [[nodiscard]]
        constexpr auto operator<=>(const fixed&) const noexcept = default;
    };

    template<class T, size_t FractBits>
    constexpr fixed<T, FractBits> operator - (const fixed<T, FractBits>& right) noexcept
    {
        return { -right.value };
    }

    template<class T, size_t FractBits>
    constexpr fixed<T, FractBits> operator + (const fixed<T, FractBits>& left, const fixed<T, FractBits>& right) noexcept
    {
        return { left.value + right.value };
    }

    template<class T, size_t FractBits>
    constexpr fixed<T, FractBits> operator - (const fixed<T, FractBits>& left, const fixed<T, FractBits>& right) noexcept
    {
        return { left.value - right.value };
    }

    template<class IntT, class T, size_t FractBits>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator + (const IntT& left, const fixed<T, FractBits>& right) noexcept
    {
        return fixed<T, FractBits>::instance(left) + right;
    }

    template<class IntT, class T, size_t FractBits>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator - (const IntT& left, const fixed<T, FractBits>& right) noexcept
    {
        return fixed<T, FractBits>::instance(left) - right;
    }

    template<class T, size_t FractBits, class IntT>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator + (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return right + left;
    }

    template<class T, size_t FractBits, class IntT>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator - (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return left - fixed<T, FractBits>::instance(right);
    }

    template<class IntT, class T, size_t FractBits>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator * (const IntT& left, const fixed<T, FractBits>& right) noexcept
    {
        return { left * right.value };
    }

    template<class T, size_t FractBits, class IntT>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator * (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return right * left;
    }

    template<class T, size_t FractBits, class IntT>
    constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator / (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return { left.value / right };
    }
}
