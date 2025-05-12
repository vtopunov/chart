#pragma once

#include <compare>

#include <core/narrow.h>


namespace font
{
    template<class T, size_t FractBits>
    struct fixed
    {
        using value_type = T;
        static constexpr size_t fract_bits{ FractBits };

        static_assert(std::is_integral_v<value_type>);
        static_assert(fract_bits <= numeric_digits_v<value_type>);

        value_type value;

        template<class Int>
        [[nodiscard]] static constexpr std::enable_if_t<std::is_integral_v<Int>, fixed> instance(Int int_value) noexcept
        {
            return { numeric_cast<value_type>(int_value) << fract_bits };
        }

        template<class NewT>
        [[nodiscard]] static constexpr fixed instance(fixed<NewT, FractBits> fix_value) noexcept
        {
            return { narrow<NewT>(fix_value.value) };
        }

        [[nodiscard]]
        constexpr value_type discard_fraction() const noexcept
        {
            return value >> fract_bits;
        }

        [[nodiscard]]
        constexpr value_type fraction() const noexcept
        {
            constexpr value_type one{ 1 };
            constexpr value_type mask{ (one << fract_bits) - one };
            return value & mask;
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

    template<class Out, class T, size_t FractBits>
    [[nodiscard]] constexpr Out trunc_to(const fixed<T, FractBits>& fp) noexcept
    {
        static_assert(std::is_integral_v<Out>);
        return narrow<Out>(fp.discard_fraction());
    }

    template<class Out, class T, size_t FractBits>
    [[nodiscard]] constexpr Out ceil_to(const fixed<T, FractBits>& fp) noexcept
    {
        using fixed_type = fixed<T, FractBits>;

        constexpr auto ceil_fraction = [] (const fixed_type& fp) noexcept
        {
            constexpr auto has_fraction = [] (const fixed_type& fp) noexcept
            {
                return !!fp.fraction();
            };

            if constexpr (std::disjunction_v<std::is_unsigned<Out>, std::is_unsigned<T>>)
            {
                return has_fraction(fp);
            }
            else
            {
                static_assert(std::is_integral_v<Out>);
                return is_positive(fp) && has_fraction(fp);
            }
        };

        return narrow<Out>(fp.discard_fraction() + ceil_fraction(fp));
    }


    template<class T, size_t FractBits>
    [[nodiscard]] constexpr fixed<T, FractBits> operator - (const fixed<T, FractBits>& right) noexcept
    {
        return { -right.value };
    }

    template<class T, size_t FractBits>
    [[nodiscard]] constexpr fixed<T, FractBits> operator + (const fixed<T, FractBits>& left, const fixed<T, FractBits>& right) noexcept
    {
        return { left.value + right.value };
    }

    template<class T, size_t FractBits>
    [[nodiscard]] constexpr fixed<T, FractBits> operator - (const fixed<T, FractBits>& left, const fixed<T, FractBits>& right) noexcept
    {
        return { left.value - right.value };
    }

    template<class IntT, class T, size_t FractBits>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator + (const IntT& left, const fixed<T, FractBits>& right) noexcept
    {
        return fixed<T, FractBits>::instance(left) + right;
    }

    template<class IntT, class T, size_t FractBits>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator - (const IntT& left, const fixed<T, FractBits>& right) noexcept
    {
        return fixed<T, FractBits>::instance(left) - right;
    }

    template<class T, size_t FractBits, class IntT>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator + (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return right + left;
    }

    template<class T, size_t FractBits, class IntT>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator - (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return left - fixed<T, FractBits>::instance(right);
    }

    template<class IntT, class T, size_t FractBits>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator * (const IntT& left, const fixed<T, FractBits>& right) noexcept
    {
        return { left * right.value };
    }

    template<class T, size_t FractBits, class IntT>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator * (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return right * left;
    }

    template<class T, size_t FractBits, class IntT>
    [[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<IntT>, fixed<T, FractBits>> operator / (const fixed<T, FractBits>& left, const IntT& right) noexcept
    {
        return { left.value / right };
    }
}
