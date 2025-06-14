#pragma once

#include <core/math.h>


namespace private_detail_zero
{
    template<class T>
    using decl_zero_value_type_t = typename T::zero_value_type;

    template<class T>
    using remove_zero_t = detected_or_t<T, decl_zero_value_type_t, T>;

    template<class T>
    using is_zero_type = is_detected<decl_zero_value_type_t, T>;

    template<class T>
    constexpr auto is_zero_type_v = is_zero_type<T>::value;

    namespace private_detail_zero_type
    {
        template<class T, class = void>
        struct zero_type1
        {
            using is_zero_constructible_constant_type = std::false_type;
        };

        template<class T>
        struct zero_type1<T, std::enable_if_t<std::is_default_constructible_v<T> > >
        {
            using is_zero_constructible_constant_type = std::true_type;

            constexpr operator T () const noexcept
            {
                return {};
            }
        };

        template<class T, class = void>
        struct zero_type0 : zero_type1<T>
        {};

        template<class T>
        struct zero_type0<T, std::void_t<decltype(T::zero())>>
        {
            using is_zero_constructible_constant_type = std::true_type;

            constexpr operator T () const noexcept
            {
                return T::zero();
            }
        };

        template<class T>
        using is_zero_constructible = typename zero_type0<T>::is_zero_constructible_constant_type;

        template<class T>
        constexpr bool is_zero_constructible_v = is_zero_constructible<T>::value;

        template<class T>
        struct zero_type : zero_type0<T>
        {
            static_assert(!std::is_reference_v<T>);
            using zero_value_type = T;

            [[nodiscard]] constexpr T operator () () const noexcept
            {
                return zero_type::operator T();
            }
        };

        template<>
        struct zero_type<void>
        {
            using zero_value_type = void;

            template<class T>
            static constexpr bool is_void_zero_constructible_v = std::conjunction_v<
                std::negation<std::is_reference<T>>,
                is_zero_constructible<T>
            >;

            template<class T, std::enable_if_t<is_void_zero_constructible_v<T>, int> = 0>
            constexpr operator T () const noexcept
            {
                constexpr zero_type0<T> zero_value{};
                return zero_value;
            }
        };

        static_assert(is_zero_type_v<zero_type<void>>);

        template<class T = void>
        using zero_t = zero_type<std::remove_cvref_t<T>>;

        template<class T = void>
        constexpr zero_t<T> zero_v{};
    }

    namespace private_detail_compare
    {
        template<class T>
        [[nodiscard]] constexpr enable_if_detected_t<decl_eq_op_t, T> eq_op(const T& left, const T& right) noexcept
        {
            return left == right;
        }

        template<class T>
        [[nodiscard]] constexpr enable_if_detected_t<decl_neq_op_t, T> neq_op(const T& left, const T& right) noexcept
        {
            return left != right;
        }

        template<class T>
        [[nodiscard]] constexpr enable_if_detected_t<decl_less_op_t, T> less_op(const T& left, const T& right) noexcept
        {
            return left < right;
        }
    }

    namespace private_detail_cmp_zero
    {
        using private_detail_zero_type::is_zero_constructible;
        using private_detail_zero_type::zero_v;
        using namespace private_detail_compare;

        template<class T, template<class, class> class Op>
        using op_result_t = typename std::enable_if_t<std::conjunction_v<std::negation<is_zero_type<T>>, is_zero_constructible<T>>, enable_if_detected<Op, T, T>>::type;

        template<class T>
        using eq_op_result_t = op_result_t<T, decl_eq_op_t>;

        template<class T>
        using neq_op_result_t = op_result_t<T, decl_neq_op_t>;

        template<class T>
        [[nodiscard]] constexpr eq_op_result_t<T> eqz(const T& value) noexcept
        {
            return eq_op<T>(value, zero_v<T>);
        }

        template<class T>
        [[nodiscard]] constexpr neq_op_result_t<T> neqz(const T& value) noexcept
        {
            return neq_op<T>(value, zero_v<T>);
        }
    }

    namespace private_detail_is_eqz
    {
        using namespace ordered_overload;
        using private_detail_cmp_zero::eqz;
        using private_detail_cmp_zero::neqz;

        template<class T>
        [[nodiscard]] constexpr auto is_eqz_helper(const T& value, _order<_2>) noexcept
            -> decltype(!neqz(value))
        {
            return !neqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto is_eqz_helper(const T& value, _order<_1>) noexcept
            -> decltype(eqz(value))
        {
            return eqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto is_eqz_helper(const T& value, _order<_0>) noexcept
            -> decltype(!value)
        {
            return !value;
        }

        template<class T>
        [[nodiscard]] constexpr auto is_eqz(const T& value) noexcept
            -> decltype(is_eqz_helper(value, _start))
        {
            return is_eqz_helper(value, _start);
        }
    }

    namespace private_detail_is_neqz
    {
        using namespace ordered_overload;
        using private_detail_cmp_zero::eqz;
        using private_detail_cmp_zero::neqz;

        template<class T>
        [[nodiscard]] constexpr auto is_neqz_helper(const T& value, _order<_2>) noexcept
            -> decltype(!eqz(value))
        {
            return !eqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto is_neqz_helper(const T& value, _order<_1>) noexcept
            -> decltype(neqz(value))
        {
            return neqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto is_neqz_helper(const T& value, _order<_0>) noexcept
            -> decltype(!!value)
        {
            return !!value;
        }

        template<class T>
        [[nodiscard]] constexpr auto is_neqz(const T& value) noexcept
            -> decltype(is_neqz_helper(value, _start))
        {
            return is_neqz_helper(value, _start);
        }
    }


    namespace private_detail_zero_type
    {
        using private_detail_is_eqz::is_eqz;
        using private_detail_is_neqz::is_neqz;

        using void_zero_type_t = zero_type<void>;

        template<class T>
        [[nodiscard]] constexpr auto operator == (const T& value, void_zero_type_t) noexcept ->
            decltype(is_eqz(value))
        {
            return is_eqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator == (void_zero_type_t, const T& value) noexcept ->
            decltype(is_eqz(value))
        {
            return is_eqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator != (const T& value, void_zero_type_t) noexcept ->
            decltype(is_neqz(value))
        {
            return is_neqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator != (void_zero_type_t, const T& value) noexcept ->
            decltype(is_neqz(value))
        {
            return is_neqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator == (const T& value, zero_type<T>) noexcept ->
            decltype(is_eqz(value))
        {
            return is_eqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator == (zero_type<T>, const T& value) noexcept ->
            decltype(is_eqz(value))
        {
            return is_eqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator != (const T& value, zero_type<T>) noexcept ->
            decltype(is_neqz(value))
        {
            return is_neqz(value);
        }

        template<class T>
        [[nodiscard]] constexpr auto operator != (zero_type<T>, const T& value) noexcept ->
            decltype(is_neqz(value))
        {
            return is_neqz(value);
        }
    }
}

using private_detail_zero::remove_zero_t;
using private_detail_zero::is_zero_type;
using private_detail_zero::is_zero_type_v;
using private_detail_zero::private_detail_zero_type::is_zero_constructible;
using private_detail_zero::private_detail_zero_type::is_zero_constructible_v;
using private_detail_zero::private_detail_zero_type::zero_t;
using private_detail_zero::private_detail_zero_type::zero_v;
using private_detail_zero::private_detail_is_eqz::is_eqz;
using private_detail_zero::private_detail_is_neqz::is_neqz;

template<class T>
using tr_zero = std::integral_constant<T, zero_v<T> >;

template<class T>
[[nodiscard]] constexpr std::enable_if_t<is_zero_constructible_v<T>, decl_less_op_t<T>> is_positive(const T& value) noexcept
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return !!value;
    }
    else
    {
        return private_detail_zero::private_detail_compare::less_op<T>(zero_v<T>, value);
    }
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<is_zero_constructible_v<T>, decl_less_op_t<T>> is_negative(const T& value) noexcept
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return false;
    }
    else
    {
        return private_detail_zero::private_detail_compare::less_op<T>(value, zero_v<T>);
    }
}

template<class T>
using decl_is_positive_t = decltype(is_positive(std::declval<const T&>()));

template<class T>
using decl_is_negative_t = decltype(is_negative(std::declval<const T&>()));

template<class T>
using has_positive_comparison = is_detected<decl_is_positive_t, T>;  

template<class T>
using has_negative_comparison = is_detected<decl_is_negative_t, T>;  

template<class T>
constexpr bool has_positive_comparison_v = has_positive_comparison<T>::value;  

template<class T>
constexpr bool has_negative_comparison_v = has_negative_comparison<T>::value;


template<class T>
[[nodiscard]] constexpr std::enable_if_t
<
    std::conjunction_v<has_negative_comparison<T>, has_unary_munis_op<T>>,
    T
> u_abs(const T& value) noexcept
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